#include "httplib.h"
#include "Feedback.h"
#include "Constants.h"
#include "Session.h"
#include "TextAnalyzer.h"
#include "Filters.h"
#include "FileHandler.h"
#include "UIComponents.h"
#include "Logger.h"
#include <sstream>
#include <fstream>
#include <algorithm>
#include <ctime>
#include <iomanip>

static std::map<std::string, std::vector<Feedback>> fil_data;
static TextAnalyzer textAnalyzer;
static Filters filters;
static FileHandler fileHandler;

static std::string getSessionId(const httplib::Request& req) {
    const auto it = req.headers.find("Cookie");
    if (it == req.headers.end()) {
        return "default";
    }

    const std::string key = "sid=";
    const auto pos = it->second.find(key);
    if (pos == std::string::npos) {
        return "default";
    }

    const auto start = pos + key.size();
    const auto end = it->second.find(';', start);
    return it->second.substr(start, end == std::string::npos ? std::string::npos : end - start);
}

// URL decode utility
static std::string urlDecode(const std::string& str) {
    std::string result;
    for (size_t i = 0; i < str.size(); i++) {
        if (str[i] == '%' && i + 2 < str.size()) {
            int val;
            std::istringstream iss(str.substr(i + 1, 2));
            if (iss >> std::hex >> val) {
                result += static_cast<char>(val);
                i += 2;
            } else {
                result += str[i];
            }
        } else if (str[i] == '+') {
            result += ' ';
        } else {
            result += str[i];
        }
    }
    return result;
}

// Parse form body
static std::map<std::string, std::string> parseForm(const std::string& body) {
    std::map<std::string, std::string> params;
    std::istringstream stream(body);
    std::string pair;
    while (std::getline(stream, pair, '&')) {
        auto eq = pair.find('=');
        if (eq != std::string::npos) {
            params[urlDecode(pair.substr(0, eq))] = urlDecode(pair.substr(eq + 1));
        }
    }
    return params;
}

static std::string getCurrentTimestamp() {
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

static std::string trimAsciiWhitespace(const std::string& text) {
    const auto start = text.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    const auto end = text.find_last_not_of(" \t\r\n");
    return text.substr(start, end - start + 1);
}

// Escape HTML
static std::string escapeHtml(const std::string& s) {
    std::string out;
    for (char c : s) {
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            default: out += c;
        }
    }
    return out;
}

// Render HTML page
static std::string renderPage(const std::string& success,
                               const std::string& warning,
                               const std::string& error,
                               const std::map<std::string, int>& sentimentResults,
                               const std::map<std::string, int>& keywordResults,
                               const std::vector<Feedback>& feedbacks) {
    std::ostringstream html;
    html << R"(<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Feedback Analyzer</title>
    <style>
        body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; margin: 0; padding: 20px; background-color: #f5f5f5; }
        .container { max-width: 1200px; margin: 0 auto; background-color: white; padding: 30px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
        h1 { color: #333; text-align: center; margin-bottom: 30px; }
        .section { margin-bottom: 30px; padding: 20px; border: 1px solid #ddd; border-radius: 5px; }
        .form-group { margin-bottom: 15px; }
        label { display: block; margin-bottom: 5px; font-weight: bold; color: #555; }
        input[type="text"], textarea, select { width: 100%; padding: 10px; border: 1px solid #ddd; border-radius: 4px; font-size: 14px; box-sizing: border-box; }
        textarea { height: 100px; resize: vertical; }
        button { background-color: #007bff; color: white; padding: 10px 20px; border: none; border-radius: 4px; cursor: pointer; font-size: 14px; margin-right: 10px; }
        button:hover { background-color: #0056b3; }
        .btn-success { background-color: #28a745; }
        .btn-success:hover { background-color: #1e7e34; }
        .alert-success { background-color: #d4edda; border-color: #c3e6cb; color: #155724; padding: 10px; border-radius: 4px; }
        .alert-warning { background-color: #fff3cd; border-color: #ffeaa7; color: #856404; padding: 10px; border-radius: 4px; }
        .alert-danger { background-color: #f8d7da; border-color: #f5c6cb; color: #721c24; padding: 10px; border-radius: 4px; }
        .stats { display: flex; justify-content: space-around; margin: 20px 0; }
        .stat-item { text-align: center; padding: 15px; background-color: #f8f9fa; border-radius: 5px; flex: 1; margin: 0 10px; }
        .stat-number { font-size: 24px; font-weight: bold; color: #007bff; }
        .stat-label { color: #666; margin-top: 5px; }
    </style>
</head>
<body>
<div class="container">
    <h1>Feedback Analyzer</h1>
    <p style="text-align: center; color: #666;">)" << u8"고객 피드백 분석 시스템" << R"(</p>)";

    if (!success.empty()) {
        html << R"(<p class="alert alert-success">)" << getCurrentTimestamp() << " : " << escapeHtml(success) << "</p>";
    }

    // Input Section
    html << R"(
    <div class="section">
        <h3>)" << u8"피드백 입력" << R"(</h3>
        <form action="/analyze" method="post">
            <div class="form-group">
                <label for="text">)" << u8"피드백 텍스트:" << R"(</label>
                <textarea id="text" name="text" placeholder=")" << u8"피드백을 입력하세요..." << R"("></textarea>
            </div>
            <button type="submit">)" << u8"입력하기" << R"(</button>
        </form>
    </div>)";

    // File Upload Section
    html << R"(
    <div class="section">
        <h3>)" << u8"CSV 파일 업로드 (선택사항)" << R"(</h3>
        <form action="/upload" method="post" enctype="multipart/form-data">
            <div class="form-group">
                <label for="file">)" << u8"CSV 파일 선택:" << R"(</label>
                <input type="file" id="file" name="file" accept=".csv">
            </div>
            <button type="submit">)" << u8"업로드" << R"(</button>
        </form>
    </div>)";

    // Filter Section
    const auto& cats = UIComponents::getCategories();
    html << R"(
    <div class="section">
        <h3>)" << u8"피드백 분석" << R"(</h3>
        <form action="/filter" method="post">
            <div class="form-group">
                <label for="sentiment">)" << u8"감정 필터:" << R"(</label>
                <select id="sentiment" name="sentiment">
                    <option value=")" << u8"전체" << R"(">)" << u8"전체" << R"(</option>
                    <option value=")" << u8"긍정" << R"(">)" << u8"긍정" << R"(</option>
                    <option value=")" << u8"중립" << R"(">)" << u8"중립" << R"(</option>
                    <option value=")" << u8"부정" << R"(">)" << u8"부정" << R"(</option>
                </select>
            </div>
            <div class="form-group">
                <label for="keyword">)" << u8"키워드 필터:" << R"(</label>
                <select id="keyword" name="keyword">
                    <option value=")" << u8"전체" << R"(">)" << u8"전체" << R"(</option>)";
    for (const auto& cat : cats) {
        html << R"(<option value=")" << cat << R"(">)" << cat << "</option>";
    }
    html << R"(
                </select>
            </div>
            <button type="submit">)" << u8"분  석" << R"(</button>
        </form>
    </div>)";

    if (!warning.empty()) {
        html << R"(<p class="alert alert-warning">)" << escapeHtml(warning) << "</p>";
    }

    // Results Section
    if (!sentimentResults.empty() || !keywordResults.empty()) {
        html << R"(<div class="section"><h3>)" << u8"분석 결과" << "</h3>";
        if (!sentimentResults.empty()) {
            html << "<h4>" << u8"감정 분포" << R"(</h4><div class="stats">)";
            for (const auto& entry : sentimentResults) {
                html << R"(<div class="stat-item"><div class="stat-number">)" << entry.second
                     << R"(</div><div class="stat-label">)" << entry.first << "</div></div>";
            }
            html << "</div>";
        }
        if (!keywordResults.empty()) {
            html << "<h4>" << u8"키워드 분포" << R"(</h4><div class="stats">)";
            for (const auto& entry : keywordResults) {
                html << R"(<div class="stat-item"><div class="stat-number">)" << entry.second
                     << R"(</div><div class="stat-label">)" << entry.first << "</div></div>";
            }
            html << "</div>";
        }
        html << R"(<a href="/download"><button class="btn-success">)" << u8"결과 다운로드" << "</button></a></div>";
    }

    if (!error.empty()) {
        html << R"(<p class="alert alert-danger">)" << escapeHtml(error) << "</p>";
    }

    html << "</div></body></html>";
    return html.str();
}

static void setHtmlResponse(httplib::Response& res, const std::string& html) {
    res.set_content(html, "text/html; charset=UTF-8");
}

struct AnalysisResult {
    std::map<std::string, int> sentiment;
    std::map<std::string, int> keywords;
};

static AnalysisResult analyzeFeedbacks(const std::vector<Feedback>& feedbacks) {
    return {textAnalyzer.sent(feedbacks), textAnalyzer.kw(feedbacks)};
}

int main() {
    Constants::init();
    Filters::initFilterKeywords();

    httplib::Server svr;

    // GET /
    svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
        Session::initSessionStateUgly();
        auto& feedbacks = Session::getOldDataFromSession("current_feedbacks");
        std::string html = renderPage(u8"피드백 분석기 시작", "", "", {}, {}, feedbacks);
        setHtmlResponse(res, html);
    });

    // POST /analyze
    svr.Post("/analyze", [](const httplib::Request& req, httplib::Response& res) {
        try {
            auto& feedbacks = Session::getCurrentFeedbacks(getSessionId(req));
            auto params = parseForm(req.body);
            std::string text = params["text"];
            text = trimAsciiWhitespace(text);

            if (text.empty()) {
                std::string html = renderPage("", u8"유효한 입력을 입력해주세요.", "", {}, {}, feedbacks);
                setHtmlResponse(res, html);
                return;
            }

            feedbacks.push_back(Feedback(text));

            for (const auto& fb : feedbacks) {
                Logger::logInfo(fb.getText());
            }

            Logger::logInfo(u8"현재 " + std::to_string(feedbacks.size()) + u8"개의 피드백이 입력되었습니다.");

            std::string success = std::to_string(feedbacks.size()) + u8"개의 피드백이 입력되었습니다.";
            AnalysisResult analysis;

            if (!feedbacks.empty()) {
                analysis = analyzeFeedbacks(feedbacks);
                Logger::logInfo(u8"감성 분석 완료");
                Logger::logInfo(u8"키워드 분석 완료");
            }

            std::string html = renderPage(success, "", "", analysis.sentiment, analysis.keywords, feedbacks);
            setHtmlResponse(res, html);
        } catch (const std::exception& e) {
            Logger::logError(std::string(u8"오류 발생: ") + e.what());
            std::string html = renderPage("", "", u8"처리 중 오류가 발생했습니다.", {}, {}, {});
            setHtmlResponse(res, html);
        }
    });

    // POST /upload
    svr.Post("/upload", [](const httplib::Request& req, httplib::Response& res) {
        try {
            auto& feedbacks = Session::getCurrentFeedbacks(getSessionId(req));
            if (!req.form.has_file("file")) {
                std::string html = renderPage("", u8"파일이 선택되지 않았습니다.", "", {}, {}, feedbacks);
                setHtmlResponse(res, html);
                return;
            }

            const auto previousCount = feedbacks.size();
            if (req.form.has_file("file")) {
                const auto file = req.form.get_file("file");
                if (!file.content.empty()) {
                    bool firstLine = true;
                    size_t textColumn = 0;
                    for (const auto& fields : FileHandler::parseCsvRecords(file.content)) {
                        if (fields.empty() || (fields.size() == 1 && fields[0].empty())) continue;
                        if (firstLine) {
                            firstLine = false;
                            auto header = std::find(fields.begin(), fields.end(), "text");
                            if (header != fields.end()) {
                                textColumn = static_cast<size_t>(std::distance(fields.begin(), header));
                                continue;
                            }
                        }
                        if (fields.size() > textColumn && !fields[textColumn].empty()) {
                            feedbacks.push_back(Feedback(fields[textColumn]));
                        }
                    }
                    Logger::logInfo(u8"파일이 성공적으로 업로드되었습니다.");
                }
            }
            if (feedbacks.size() == previousCount) {
                std::string html = renderPage("", u8"유효한 CSV 피드백이 없습니다.", "", {}, {}, feedbacks);
                setHtmlResponse(res, html);
                return;
            }
            std::string success = std::to_string(feedbacks.size()) + u8"개의 피드백이 입력되었습니다.";
            std::string html = renderPage(success, "", "", {}, {}, feedbacks);
            setHtmlResponse(res, html);
        } catch (const std::exception& e) {
            Logger::logError(std::string(u8"파일 업로드 오류: ") + e.what());
            std::string html = renderPage("", "", u8"파일 업로드 중 오류가 발생했습니다.", {}, {}, {});
            setHtmlResponse(res, html);
        }
    });

    // POST /filter
    svr.Post("/filter", [](const httplib::Request& req, httplib::Response& res) {
        try {
            const auto sessionId = getSessionId(req);
            auto& feedbacks = Session::getCurrentFeedbacks(sessionId);
            auto params = parseForm(req.body);
            std::string sentiment = params["sentiment"];
            std::string keyword = params["keyword"];

            if (!feedbacks.empty()) {
                auto filtered = filters.fil(feedbacks, sentiment, keyword);
                if (!filtered.empty()) {
                    fil_data[sessionId] = filtered;
                    auto analysis = analyzeFeedbacks(filtered);
                    Logger::logInfo(u8"필터링 결과: " + std::to_string(filtered.size()) + u8"개의 피드백");
                    std::string html = renderPage("", "", "", analysis.sentiment, analysis.keywords, filtered);
                    setHtmlResponse(res, html);
                } else {
                    fil_data.erase(sessionId);
                    Logger::logWarning(u8"필터링 결과가 없습니다.");
                    std::string html = renderPage("", u8"필터링 결과가 없습니다.", "", {}, {}, {});
                    setHtmlResponse(res, html);
                }
            } else {
                Logger::logWarning(u8"분석할 피드백이 없습니다.");
                std::string html = renderPage("", u8"분석할 피드백이 없습니다.", "", {}, {}, {});
                setHtmlResponse(res, html);
            }
        } catch (const std::exception& e) {
            Logger::logError(std::string(u8"오류 발생: ") + e.what());
            std::string html = renderPage("", "", u8"처리 중 오류가 발생했습니다.", {}, {}, {});
            setHtmlResponse(res, html);
        }
    });

    // GET /download
    svr.Get("/download", [](const httplib::Request& req, httplib::Response& res) {
        const auto sessionId = getSessionId(req);
        const auto filtered = fil_data.find(sessionId);
        if (filtered == fil_data.end() || filtered->second.empty()) {
            res.set_content("", "text/csv; charset=UTF-8");
            return;
        }

        std::ostringstream csv;
        // UTF-8 BOM
        csv << "\xEF\xBB\xBF";
        csv << "text\n";
        for (const auto& iter : filtered->second) {
            csv << FileHandler::escapeCsvField(iter.getText()) << "\n";
        }
        res.set_header("Content-Disposition", "attachment; filename=\"filtered_feedback.csv\"");
        res.set_content(csv.str(), "text/csv; charset=UTF-8");
    });

    Logger::logInfo(u8"서버가 http://localhost:8080 에서 시작됩니다.");
    if (!svr.listen("0.0.0.0", 8080)) {
        Logger::logError(u8"서버 시작 실패: 포트 8080 바인딩을 확인하세요.");
        return 1;
    }

    return 0;
}
