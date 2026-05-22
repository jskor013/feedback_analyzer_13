#pragma once

#include "CsvFeedbackExporter.h"
#include "CsvFeedbackImporter.h"
#include "Feedback.h"
#include "FeedbackPageRenderer.h"
#include "FeedbackPageViewModel.h"
#include "FilteredResultStore.h"
#include "Filters.h"
#include "Logger.h"
#include "Session.h"
#include "TextAnalyzer.h"
#include "httplib.h"

#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace RequestHandlers {

inline TextAnalyzer& textAnalyzer() {
    static TextAnalyzer analyzer;
    return analyzer;
}

inline Filters& filters() {
    static Filters filterEngine;
    return filterEngine;
}

inline std::string getSessionId(const httplib::Request& req) {
    const auto it = req.headers.find("Cookie");
    if (it == req.headers.end()) {
        return Session::DEFAULT_SESSION_ID;
    }

    const std::string key = "sid=";
    const auto pos = it->second.find(key);
    if (pos == std::string::npos) {
        return Session::DEFAULT_SESSION_ID;
    }

    const auto start = pos + key.size();
    const auto end = it->second.find(';', start);
    return it->second.substr(start, end == std::string::npos ? std::string::npos : end - start);
}

inline std::string urlDecode(const std::string& str) {
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

inline std::map<std::string, std::string> parseForm(const std::string& body) {
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

inline std::string trimAsciiWhitespace(const std::string& text) {
    const auto start = text.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    const auto end = text.find_last_not_of(" \t\r\n");
    return text.substr(start, end - start + 1);
}

inline std::string renderPage(const std::string& success,
                              const std::string& warning,
                              const std::string& error,
                              const std::map<std::string, int>& sentimentResults,
                              const std::map<std::string, int>& keywordResults,
                              const std::vector<Feedback>& feedbacks) {
    return FeedbackPageRenderer::render({success, warning, error, sentimentResults, keywordResults, feedbacks});
}

inline void setHtmlResponse(httplib::Response& res, const std::string& html) {
    res.set_content(html, "text/html; charset=UTF-8");
}

struct AnalysisResult {
    std::map<std::string, int> sentiment;
    std::map<std::string, int> keywords;
};

inline AnalysisResult analyzeFeedbacks(const std::vector<Feedback>& feedbacks) {
    return {textAnalyzer().sent(feedbacks), textAnalyzer().kw(feedbacks)};
}

inline void handleIndex(const httplib::Request&, httplib::Response& res) {
    Session::initSessionStateUgly();
    auto& feedbacks = Session::getOldDataFromSession("current_feedbacks");
    setHtmlResponse(res, renderPage(u8"피드백 분석기 시작", "", "", {}, {}, feedbacks));
}

inline void handleAnalyze(const httplib::Request& req, httplib::Response& res) {
    try {
        auto& feedbacks = Session::getCurrentFeedbacks(getSessionId(req));
        auto params = parseForm(req.body);
        std::string text = trimAsciiWhitespace(params["text"]);

        if (text.empty()) {
            setHtmlResponse(res, renderPage("", u8"유효한 입력을 입력해주세요.", "", {}, {}, feedbacks));
            return;
        }

        feedbacks.push_back(Feedback(text));
        Logger::logInfo(u8"현재 " + std::to_string(feedbacks.size()) + u8"개의 피드백이 입력되었습니다.");

        const std::string success = std::to_string(feedbacks.size()) + u8"개의 피드백이 입력되었습니다.";
        AnalysisResult analysis;
        if (!feedbacks.empty()) {
            analysis = analyzeFeedbacks(feedbacks);
            Logger::logInfo(u8"감성 분석 완료");
            Logger::logInfo(u8"키워드 분석 완료");
        }

        setHtmlResponse(res, renderPage(success, "", "", analysis.sentiment, analysis.keywords, feedbacks));
    } catch (const std::exception& e) {
        Logger::logError(std::string(u8"오류 발생: ") + e.what());
        setHtmlResponse(res, renderPage("", "", u8"처리 중 오류가 발생했습니다.", {}, {}, {}));
    }
}

inline void handleUpload(const httplib::Request& req, httplib::Response& res) {
    try {
        auto& feedbacks = Session::getCurrentFeedbacks(getSessionId(req));
        if (!req.form.has_file("file")) {
            setHtmlResponse(res, renderPage("", u8"파일이 선택되지 않았습니다.", "", {}, {}, feedbacks));
            return;
        }

        const auto previousCount = feedbacks.size();
        const auto file = req.form.get_file("file");
        const auto importResult = CsvFeedbackImporter::importFeedbacks(file.content);
        feedbacks.insert(feedbacks.end(), importResult.feedbacks.begin(), importResult.feedbacks.end());
        if (!file.content.empty()) {
            Logger::logInfo(u8"파일이 성공적으로 업로드되었습니다.");
        }

        if (feedbacks.size() == previousCount) {
            setHtmlResponse(res, renderPage("", u8"유효한 CSV 피드백이 없습니다.", "", {}, {}, feedbacks));
            return;
        }

        const std::string success = std::to_string(feedbacks.size()) + u8"개의 피드백이 입력되었습니다.";
        setHtmlResponse(res, renderPage(success, "", "", {}, {}, feedbacks));
    } catch (const std::exception& e) {
        Logger::logError(std::string(u8"파일 업로드 오류: ") + e.what());
        setHtmlResponse(res, renderPage("", "", u8"파일 업로드 중 오류가 발생했습니다.", {}, {}, {}));
    }
}

inline void handleFilter(const httplib::Request& req, httplib::Response& res) {
    try {
        const auto sessionId = getSessionId(req);
        auto& feedbacks = Session::getCurrentFeedbacks(sessionId);
        auto params = parseForm(req.body);
        std::string sentiment = params["sentiment"];
        std::string keyword = params["keyword"];

        if (!feedbacks.empty()) {
            auto filtered = filters().fil(feedbacks, sentiment, keyword);
            if (!filtered.empty()) {
                FilteredResultStore::save(sessionId, filtered);
                auto analysis = analyzeFeedbacks(filtered);
                Logger::logInfo(u8"필터링 결과: " + std::to_string(filtered.size()) + u8"개의 피드백");
                setHtmlResponse(res, renderPage("", "", "", analysis.sentiment, analysis.keywords, filtered));
            } else {
                FilteredResultStore::clear(sessionId);
                Logger::logWarning(u8"필터링 결과가 없습니다.");
                setHtmlResponse(res, renderPage("", u8"필터링 결과가 없습니다.", "", {}, {}, {}));
            }
        } else {
            Logger::logWarning(u8"분석할 피드백이 없습니다.");
            setHtmlResponse(res, renderPage("", u8"분석할 피드백이 없습니다.", "", {}, {}, {}));
        }
    } catch (const std::exception& e) {
        Logger::logError(std::string(u8"오류 발생: ") + e.what());
        setHtmlResponse(res, renderPage("", "", u8"처리 중 오류가 발생했습니다.", {}, {}, {}));
    }
}

inline void handleDownload(const httplib::Request& req, httplib::Response& res) {
    const auto sessionId = getSessionId(req);
    const auto filtered = FilteredResultStore::find(sessionId);
    if (filtered == nullptr) {
        res.set_content("", "text/csv; charset=UTF-8");
        return;
    }

    res.set_header("Content-Disposition", "attachment; filename=\"filtered_feedback.csv\"");
    res.set_content(CsvFeedbackExporter::exportFeedbacks(*filtered), "text/csv; charset=UTF-8");
}

} // namespace RequestHandlers
