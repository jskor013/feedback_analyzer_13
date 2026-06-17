// RED CASE tests for legacy defects that were not automated in the first pass.
// This file intentionally does not modify src code. It includes main.cpp behind
// a tiny fake httplib boundary so the existing route lambdas can be exercised
// without opening a real HTTP port.

#include <functional>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

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

#define main feedback_analyzer_legacy_main
#include "../../src/cpp/main.cpp"
#undef main

namespace legacy_unautomated_red_case_runner {

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

} // namespace legacy_unautomated_red_case_runner

#define LEGACY_UNAUTO_CONCAT_INNER(a, b) a##b
#define LEGACY_UNAUTO_CONCAT(a, b) LEGACY_UNAUTO_CONCAT_INNER(a, b)
#define TEST_CASE(name, tags)                                                                     \
    static void LEGACY_UNAUTO_CONCAT(legacy_unautomated_red_case_test_, __LINE__)();              \
    namespace {                                                                                   \
    const bool LEGACY_UNAUTO_CONCAT(legacy_unautomated_red_case_registered_, __LINE__) =           \
        legacy_unautomated_red_case_runner::add(                                                   \
            name, &LEGACY_UNAUTO_CONCAT(legacy_unautomated_red_case_test_, __LINE__));             \
    }                                                                                             \
    static void LEGACY_UNAUTO_CONCAT(legacy_unautomated_red_case_test_, __LINE__)()

#define REQUIRE(expression)                                                                       \
    do {                                                                                          \
        if (!(expression)) {                                                                      \
            throw std::runtime_error("Requirement failed: " #expression);                         \
        }                                                                                         \
    } while (false)

namespace {

void resetLegacyState() {
    Session::updateCurrentFeedbacks({});
    fil_data.clear();
    httplib::Server::reset();
}

void bootLegacyRoutes(bool listenResult = true) {
    httplib::Server::reset();
    httplib::Server::nextListenResult = listenResult;
    (void)feedback_analyzer_legacy_main();
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

} // namespace

TEST_CASE("RED-01: empty filter result clears stale download data", "[red][download]") {
    resetLegacyState();
    bootLegacyRoutes();

    const std::string feedback = u8"이 상품은 좋아요";
    (void)invokePost("/analyze", "text=" + feedback);
    (void)invokePost("/filter", u8"sentiment=전체&keyword=전체");

    const auto beforeEmptyFilter = invokeGet("/download");
    REQUIRE(contains(beforeEmptyFilter.body, feedback));

    (void)invokePost("/filter", u8"sentiment=부정&keyword=전체");
    const auto afterEmptyFilter = invokeGet("/download");

    REQUIRE(!contains(afterEmptyFilter.body, feedback));
}

TEST_CASE("RED-02: download before filtering reports no downloadable result", "[red][download]") {
    resetLegacyState();
    bootLegacyRoutes();

    const auto response = invokeGet("/download");

    REQUIRE(response.headers.count("Content-Disposition") == 0);
    REQUIRE(response.body.empty());
}

TEST_CASE("RED-03: separate users do not share feedback state", "[red][session]") {
    resetLegacyState();
    bootLegacyRoutes();

    (void)invokePost("/analyze", u8"text=사용자A의 피드백입니다", {{"Cookie", "sid=user-a"}});
    const auto userBResponse = invokePost(
        "/filter", u8"sentiment=전체&keyword=전체", {{"Cookie", "sid=user-b"}});

    REQUIRE(contains(userBResponse.body, u8"분석할 피드백이 없습니다."));
}

TEST_CASE("RED-06: headerless CSV does not silently drop the first feedback", "[red][csv]") {
    resetLegacyState();
    bootLegacyRoutes();

    (void)invokeUpload(u8"첫번째 피드백\n두번째 피드백\n");

    REQUIRE(Session::getCurrentFeedbacks().size() == 2);
    REQUIRE(Session::getCurrentFeedbacks().at(0).getText() == u8"첫번째 피드백");
}

TEST_CASE("RED-07: CSV upload reads feedback by text column name", "[red][csv]") {
    resetLegacyState();
    bootLegacyRoutes();

    (void)invokeUpload(u8"id,text\n1,올바른 피드백\n");

    REQUIRE(Session::getCurrentFeedbacks().size() == 1);
    REQUIRE(Session::getCurrentFeedbacks().at(0).getText() == u8"올바른 피드백");
}

TEST_CASE("RED-08: CSV download escapes feedback fields containing commas", "[red][csv]") {
    resetLegacyState();
    bootLegacyRoutes();

    const std::string feedback = u8"배송은 빠르지만, 포장이 약해요";
    (void)invokeUpload(std::string(u8"text\n\"") + feedback + "\"\n");
    (void)invokePost("/filter", u8"sentiment=전체&keyword=전체");
    const auto response = invokeGet("/download");

    REQUIRE(contains(response.body, std::string("\"") + feedback + "\""));
}

TEST_CASE("RED-09: CSV parser preserves escaped quotes inside a field", "[red][csv]") {
    resetLegacyState();
    bootLegacyRoutes();

    (void)invokeUpload(u8"text\n\"제품에 \"\"흠집\"\"이 있어요\"\n");

    REQUIRE(Session::getCurrentFeedbacks().size() == 1);
    REQUIRE(Session::getCurrentFeedbacks().at(0).getText() == u8"제품에 \"흠집\"이 있어요");
}

TEST_CASE("RED-10: CSV parser preserves newlines inside a quoted field", "[red][csv]") {
    resetLegacyState();
    bootLegacyRoutes();

    (void)invokeUpload(u8"text\n\"첫 줄\n둘째 줄\"\n");

    REQUIRE(Session::getCurrentFeedbacks().size() == 1);
    REQUIRE(Session::getCurrentFeedbacks().at(0).getText() == u8"첫 줄\n둘째 줄");
}

TEST_CASE("RED-11: whitespace-only feedback is rejected with a clear message", "[red][validation]") {
    resetLegacyState();
    bootLegacyRoutes();

    const auto response = invokePost("/analyze", "text=+%09%0D%0A");

    REQUIRE(contains(response.body, u8"유효한 입력"));
    REQUIRE(Session::getCurrentFeedbacks().empty());
}

TEST_CASE("RED-12: upload without selecting a file is rejected with a clear message", "[red][validation]") {
    resetLegacyState();
    bootLegacyRoutes();

    const auto response = invokeUploadWithoutFile();

    REQUIRE(contains(response.body, u8"파일이 선택되지"));
}

TEST_CASE("RED-13: server start failure returns a failing exit code", "[red][server]") {
    resetLegacyState();
    httplib::Server::nextListenResult = false;

    const int exitCode = feedback_analyzer_legacy_main();

    REQUIRE(exitCode != 0);
}

int main() {
    int failed = 0;
    for (const auto& test : legacy_unautomated_red_case_runner::registry()) {
        try {
            test.fn();
            std::cout << "[PASS] " << test.name << '\n';
        } catch (const std::exception& e) {
            ++failed;
            std::cout << "[FAIL] " << test.name << " - " << e.what() << '\n';
        }
    }
    std::cout << "LEGACY_UNAUTOMATED_RED_CASE_EXIT_CODE=" << (failed == 0 ? 0 : 1) << '\n';
    return failed == 0 ? 0 : 1;
}
