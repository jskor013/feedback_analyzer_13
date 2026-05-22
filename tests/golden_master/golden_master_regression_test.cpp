// Golden Master regression tests for the refactoring phase.
// These tests exercise the HTTP routes through a fake httplib boundary so they
// can run without opening a real port.

#if defined(GOLDEN_MASTER_USE_CATCH2)
#include <catch2/catch_test_macros.hpp>
#else
#include <iostream>
#include <stdexcept>
#include <vector>
#endif

#include <functional>
#include <map>
#include <sstream>
#include <string>

#define CPPHTTPLIB_HTTPLIB_H
namespace httplib {

struct MultipartFormData {
    std::string name;
    std::string content;
    std::string filename;
    std::string content_type;
};

class MultipartFormDataMap {
public:
    bool has_file(const std::string& key) const {
        return files_.count(key) > 0;
    }

    const MultipartFormData& get_file(const std::string& key) const {
        return files_.at(key);
    }

    void set_file(const std::string& key, const MultipartFormData& file) {
        files_[key] = file;
    }

private:
    std::map<std::string, MultipartFormData> files_;
};

struct Request {
    std::string body;
    MultipartFormDataMap form;
    std::map<std::string, std::string> headers;
};

struct Response {
    int status = 200;
    std::string body;
    std::string content_type;
    std::map<std::string, std::string> headers;

    void set_content(const std::string& content, const std::string& type) {
        body = content;
        content_type = type;
    }

    void set_header(const std::string& name, const std::string& value) {
        headers[name] = value;
    }
};

class Server {
public:
    using Handler = std::function<void(const Request&, Response&)>;

    void Get(const std::string& path, Handler handler) {
        getRoutes()[path] = std::move(handler);
    }

    void Post(const std::string& path, Handler handler) {
        postRoutes()[path] = std::move(handler);
    }

    bool listen(const std::string& host, int port) {
        listenedHost = host;
        listenedPort = port;
        return nextListenResult;
    }

    static std::map<std::string, Handler>& getRoutes() {
        static std::map<std::string, Handler> routes;
        return routes;
    }

    static std::map<std::string, Handler>& postRoutes() {
        static std::map<std::string, Handler> routes;
        return routes;
    }

    static void reset() {
        getRoutes().clear();
        postRoutes().clear();
        nextListenResult = true;
        listenedHost.clear();
        listenedPort = 0;
    }

    static bool nextListenResult;
    static std::string listenedHost;
    static int listenedPort;
};

bool Server::nextListenResult = true;
std::string Server::listenedHost;
int Server::listenedPort = 0;

} // namespace httplib

#define main feedback_analyzer_golden_master_main
#include "../../src/cpp/main.cpp"
#undef main

#if !defined(GOLDEN_MASTER_USE_CATCH2)
namespace golden_master_runner {

using TestFn = void (*)();

struct TestCase {
    const char* name;
    TestFn fn;
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> tests;
    return tests;
}

inline bool add(const char* name, TestFn fn) {
    registry().push_back({name, fn});
    return true;
}

} // namespace golden_master_runner

#define GM_CONCAT_INNER(a, b) a##b
#define GM_CONCAT(a, b) GM_CONCAT_INNER(a, b)
#define TEST_CASE(name, tags)                                                                     \
    static void GM_CONCAT(golden_master_test_, __LINE__)();                                       \
    namespace {                                                                                   \
    const bool GM_CONCAT(golden_master_registered_, __LINE__) =                                   \
        golden_master_runner::add(name, &GM_CONCAT(golden_master_test_, __LINE__));               \
    }                                                                                             \
    static void GM_CONCAT(golden_master_test_, __LINE__)()
#define REQUIRE(expression)                                                                       \
    do {                                                                                          \
        if (!(expression)) {                                                                      \
            throw std::runtime_error("Requirement failed: " #expression);                         \
        }                                                                                         \
    } while (false)
#define REQUIRE_FALSE(expression) REQUIRE(!(expression))
#endif

namespace {

const std::string kPositiveShipping = u8"배송이 빠르고 서비스가 좋아요";
const std::string kNegativeShipping = u8"배송지연 때문에 너무 불편하고 실망했습니다";
const std::string kPriceValue = u8"가격이 저렴하고 가성비가 좋아요";
const std::string kQualityGood = u8"제품 품질이 좋고 마감이 훌륭합니다";
const std::string kServiceKind = u8"상담 응대가 친절해서 만족합니다";
const std::string kUsabilityBad = u8"사용법이 어렵고 설명서가 불편합니다";

class StreamCapture {
public:
    StreamCapture(std::ostream& stream) : stream_(stream), oldBuffer_(stream.rdbuf(captured_.rdbuf())) {}

    ~StreamCapture() {
        stream_.rdbuf(oldBuffer_);
    }

    std::string str() const {
        return captured_.str();
    }

private:
    std::ostream& stream_;
    std::ostringstream captured_;
    std::streambuf* oldBuffer_;
};

void resetGoldenMasterState() {
    Session::updateCurrentFeedbacks({});
    fil_data.clear();
    httplib::Server::reset();
    Logger::setDebugMode(false);
}

void bootRoutes(bool listenResult = true) {
    httplib::Server::reset();
    httplib::Server::nextListenResult = listenResult;
    (void)feedback_analyzer_golden_master_main();
}

httplib::Response invokeGet(const std::string& path,
                            const std::map<std::string, std::string>& headers = {}) {
    httplib::Request request;
    request.headers = headers;
    httplib::Response response;
    httplib::Server::getRoutes().at(path)(request, response);
    return response;
}

httplib::Response invokePost(const std::string& path,
                             const std::string& body,
                             const std::map<std::string, std::string>& headers = {}) {
    httplib::Request request;
    request.body = body;
    request.headers = headers;
    httplib::Response response;
    httplib::Server::postRoutes().at(path)(request, response);
    return response;
}

httplib::Response invokeUpload(const std::string& fileContent,
                               const std::map<std::string, std::string>& headers = {}) {
    httplib::Request request;
    request.headers = headers;
    request.form.set_file("file", {"file", fileContent, "feedback.csv", "text/csv"});
    httplib::Response response;
    httplib::Server::postRoutes().at("/upload")(request, response);
    return response;
}

httplib::Response invokeUploadWithoutFile() {
    httplib::Request request;
    httplib::Response response;
    httplib::Server::postRoutes().at("/upload")(request, response);
    return response;
}

bool contains(const std::string& haystack, const std::string& needle) {
    return haystack.find(needle) != std::string::npos;
}

std::map<std::string, std::string> sid(const std::string& sessionId) {
    return {{"Cookie", "sid=" + sessionId}};
}

std::vector<std::vector<std::string>> parseDownloadedCsv(const std::string& body) {
    std::string csv = body;
    if (csv.rfind("\xEF\xBB\xBF", 0) == 0) {
        csv.erase(0, 3);
    }
    return FileHandler::parseCsvRecords(csv);
}

} // namespace

TEST_CASE("GM-01: initial page renders the main feedback form", "[golden][render]") {
    resetGoldenMasterState();
    bootRoutes();

    const auto response = invokeGet("/");

    REQUIRE(response.content_type == "text/html; charset=UTF-8");
    REQUIRE(contains(response.body, "Feedback Analyzer"));
    REQUIRE(contains(response.body, u8"피드백 텍스트"));
    REQUIRE(contains(response.body, u8"CSV 파일 업로드"));
    REQUIRE(contains(response.body, u8"감정 필터"));
    REQUIRE(contains(response.body, u8"키워드 필터"));
}

TEST_CASE("GM-02: single direct feedback is analyzed", "[golden][input][analysis]") {
    resetGoldenMasterState();
    bootRoutes();

    const auto response = invokePost("/analyze", "text=" + kPositiveShipping);

    REQUIRE(Session::getCurrentFeedbacks().size() == 1);
    REQUIRE(contains(response.body, u8"1개의 피드백"));
    REQUIRE(contains(response.body, u8"긍정"));
}

TEST_CASE("GM-03: direct feedback accumulates inside one session", "[golden][session][analysis]") {
    resetGoldenMasterState();
    bootRoutes();

    const auto headers = sid("gm-03");
    (void)invokePost("/analyze", "text=" + kPositiveShipping, headers);
    (void)invokePost("/analyze", "text=" + kNegativeShipping, headers);
    const auto response = invokePost("/analyze", "text=" + kPriceValue, headers);

    REQUIRE(Session::getCurrentFeedbacks("gm-03").size() == 3);
    REQUIRE(contains(response.body, u8"3개의 피드백"));
    REQUIRE(contains(response.body, u8"긍정"));
    REQUIRE(contains(response.body, u8"부정"));
    REQUIRE(contains(response.body, u8"가격"));
}

TEST_CASE("GM-04: separate sessions do not share feedback", "[golden][session]") {
    resetGoldenMasterState();
    bootRoutes();

    (void)invokePost("/analyze", "text=" + kPositiveShipping, sid("user-a"));
    const auto userBResponse = invokePost("/filter", u8"sentiment=전체&keyword=전체", sid("user-b"));

    REQUIRE(contains(userBResponse.body, u8"분석할 피드백이 없습니다."));
    REQUIRE_FALSE(contains(userBResponse.body, kPositiveShipping));
}

TEST_CASE("GM-05: normal CSV upload stores all text rows", "[golden][csv][upload]") {
    resetGoldenMasterState();
    bootRoutes();

    const auto response = invokeUpload(
        std::string("text\n") + kPositiveShipping + "\n" + kPriceValue + "\n" + kQualityGood + "\n");

    REQUIRE(Session::getCurrentFeedbacks().size() == 3);
    REQUIRE(contains(response.body, u8"3개의 피드백"));
}

TEST_CASE("GM-06: CSV upload reads the text column by name", "[golden][csv][upload]") {
    resetGoldenMasterState();
    bootRoutes();

    (void)invokeUpload(std::string("id,text\n1,") + kPositiveShipping + "\n2," + kPriceValue + "\n");

    REQUIRE(Session::getCurrentFeedbacks().size() == 2);
    REQUIRE(Session::getCurrentFeedbacks().at(0).getText() == kPositiveShipping);
    REQUIRE(Session::getCurrentFeedbacks().at(1).getText() == kPriceValue);
}

TEST_CASE("GM-07: headerless CSV upload keeps the first feedback row", "[golden][csv][upload]") {
    resetGoldenMasterState();
    bootRoutes();

    (void)invokeUpload(kPositiveShipping + "\n" + kPriceValue + "\n");

    REQUIRE(Session::getCurrentFeedbacks().size() == 2);
    REQUIRE(Session::getCurrentFeedbacks().at(0).getText() == kPositiveShipping);
}

TEST_CASE("GM-08: CSV parser preserves quoted commas quotes and newlines", "[golden][csv][escaping]") {
    resetGoldenMasterState();
    bootRoutes();

    (void)invokeUpload(u8"text\n\"배송은 빠르지만, 포장이 약해요\"\n");
    (void)invokeUpload(u8"text\n\"제품에 \"\"흠집\"\"이 있어요\"\n");
    (void)invokeUpload(u8"text\n\"첫 줄\n둘째 줄\"\n");

    REQUIRE(Session::getCurrentFeedbacks().size() == 3);
    REQUIRE(Session::getCurrentFeedbacks().at(0).getText() == u8"배송은 빠르지만, 포장이 약해요");
    REQUIRE(Session::getCurrentFeedbacks().at(1).getText() == u8"제품에 \"흠집\"이 있어요");
    REQUIRE(Session::getCurrentFeedbacks().at(2).getText() == u8"첫 줄\n둘째 줄");
}

TEST_CASE("GM-09: sentiment filters match sentiment analysis rules", "[golden][sentiment][filter]") {
    resetGoldenMasterState();
    bootRoutes();

    (void)invokeUpload(std::string("text\n") + kPositiveShipping + "\n" + kNegativeShipping + "\n");

    (void)invokePost("/filter", u8"sentiment=긍정&keyword=전체");
    const auto positiveDownload = invokeGet("/download");
    REQUIRE(contains(positiveDownload.body, kPositiveShipping));
    REQUIRE_FALSE(contains(positiveDownload.body, kNegativeShipping));

    (void)invokePost("/filter", u8"sentiment=부정&keyword=전체");
    const auto negativeDownload = invokeGet("/download");
    REQUIRE(contains(negativeDownload.body, kNegativeShipping));
    REQUIRE_FALSE(contains(negativeDownload.body, kPositiveShipping));
}

TEST_CASE("GM-10: category filters match category analysis rules", "[golden][category][filter]") {
    resetGoldenMasterState();
    bootRoutes();

    (void)invokeUpload(std::string("text\n") + kPositiveShipping + "\n" + kPriceValue + "\n" +
                       kQualityGood + "\n" + kServiceKind + "\n" + kUsabilityBad + "\n");

    (void)invokePost("/filter", u8"sentiment=전체&keyword=가격");
    const auto priceDownload = invokeGet("/download");
    REQUIRE(contains(priceDownload.body, kPriceValue));
    REQUIRE_FALSE(contains(priceDownload.body, kQualityGood));

    (void)invokePost("/filter", u8"sentiment=전체&keyword=품질");
    const auto qualityDownload = invokeGet("/download");
    REQUIRE(contains(qualityDownload.body, kQualityGood));
    REQUIRE_FALSE(contains(qualityDownload.body, kPriceValue));
}

TEST_CASE("GM-11: download returns the current filter result", "[golden][download]") {
    resetGoldenMasterState();
    bootRoutes();

    (void)invokeUpload(std::string("text\n") + kPositiveShipping + "\n" + kPriceValue + "\n");
    (void)invokePost("/filter", u8"sentiment=전체&keyword=가격");
    const auto response = invokeGet("/download");

    REQUIRE(response.headers.count("Content-Disposition") == 1);
    REQUIRE(contains(response.body, kPriceValue));
    REQUIRE_FALSE(contains(response.body, kPositiveShipping));
}

TEST_CASE("GM-12: download before filtering has no attachment", "[golden][download]") {
    resetGoldenMasterState();
    bootRoutes();

    const auto response = invokeGet("/download");

    REQUIRE(response.headers.count("Content-Disposition") == 0);
    REQUIRE(response.body.empty());
}

TEST_CASE("GM-13: empty filter result clears stale download data", "[golden][download]") {
    resetGoldenMasterState();
    bootRoutes();

    (void)invokePost("/analyze", "text=" + kPositiveShipping);
    (void)invokePost("/filter", u8"sentiment=전체&keyword=전체");
    REQUIRE(contains(invokeGet("/download").body, kPositiveShipping));

    (void)invokePost("/filter", u8"sentiment=부정&keyword=전체");
    const auto staleDownload = invokeGet("/download");

    REQUIRE_FALSE(contains(staleDownload.body, kPositiveShipping));
    REQUIRE(staleDownload.headers.count("Content-Disposition") == 0);
}

TEST_CASE("GM-14: downloaded CSV escapes commas quotes and newlines", "[golden][csv][download]") {
    resetGoldenMasterState();
    bootRoutes();

    (void)invokeUpload(u8"text\n\"배송은 빠르지만, 포장이 약해요\"\n\"제품에 \"\"흠집\"\"이 있어요\"\n\"첫 줄\n둘째 줄\"\n");
    (void)invokePost("/filter", u8"sentiment=전체&keyword=전체");
    const auto response = invokeGet("/download");

    REQUIRE(contains(response.body, u8"\"배송은 빠르지만, 포장이 약해요\""));
    REQUIRE(contains(response.body, u8"\"제품에 \"\"흠집\"\"이 있어요\""));
    REQUIRE(contains(response.body, u8"\"첫 줄\n둘째 줄\""));

    const auto parsed = parseDownloadedCsv(response.body);
    REQUIRE(parsed.size() == 4);
    REQUIRE(parsed.at(1).at(0) == u8"배송은 빠르지만, 포장이 약해요");
    REQUIRE(parsed.at(2).at(0) == u8"제품에 \"흠집\"이 있어요");
    REQUIRE(parsed.at(3).at(0) == u8"첫 줄\n둘째 줄");
}

TEST_CASE("GM-15: invalid inputs show clear validation messages", "[golden][validation]") {
    resetGoldenMasterState();
    bootRoutes();

    const auto whitespace = invokePost("/analyze", "text=+%09%0D%0A");
    REQUIRE(contains(whitespace.body, u8"유효한 입력"));
    REQUIRE(Session::getCurrentFeedbacks().empty());

    const auto noFile = invokeUploadWithoutFile();
    REQUIRE(contains(noFile.body, u8"파일이 선택되지"));

    const auto emptyCsv = invokeUpload("");
    REQUIRE(contains(emptyCsv.body, u8"유효한 CSV 피드백이 없습니다."));
}

TEST_CASE("GM-16: raw HTML input is not reflected as executable markup", "[golden][render][security]") {
    resetGoldenMasterState();
    bootRoutes();

    const auto response = invokePost("/analyze", "text=%3Cscript%3Ealert(1)%3C/script%3E%22");

    REQUIRE(contains(response.body, u8"1개의 피드백"));
    REQUIRE_FALSE(contains(response.body, "<script>alert(1)</script>"));
}

TEST_CASE("GM-17: server start failure returns non-zero and logs an error", "[golden][server][logging]") {
    resetGoldenMasterState();
    httplib::Server::reset();
    httplib::Server::nextListenResult = false;
    StreamCapture stderrCapture(std::cerr);

    const int exitCode = feedback_analyzer_golden_master_main();

    REQUIRE(exitCode != 0);
    REQUIRE(contains(stderrCapture.str(), "ERROR"));
    REQUIRE(contains(stderrCapture.str(), u8"서버 시작 실패"));
}

TEST_CASE("GM-18: default logs do not print raw user feedback", "[golden][logging][privacy]") {
    resetGoldenMasterState();
    StreamCapture stdoutCapture(std::cout);
    bootRoutes();

    const std::string sensitive = u8"고객 전화번호 010-1234-5678 포함된 민감 피드백";
    (void)invokePost("/analyze", "text=" + sensitive);
    (void)invokePost("/filter", u8"sentiment=전체&keyword=전체");

    REQUIRE_FALSE(Logger::isDebugMode());
    REQUIRE_FALSE(contains(stdoutCapture.str(), sensitive));
}

#if !defined(GOLDEN_MASTER_USE_CATCH2)
int main() {
    int failed = 0;
    for (const auto& test : golden_master_runner::registry()) {
        try {
            test.fn();
            std::cout << "[PASS] " << test.name << '\n';
        } catch (const std::exception& e) {
            ++failed;
            std::cout << "[FAIL] " << test.name << " - " << e.what() << '\n';
        }
    }
    std::cout << "GOLDEN_MASTER_EXIT_CODE=" << (failed == 0 ? 0 : 1) << '\n';
    return failed == 0 ? 0 : 1;
}
#endif
