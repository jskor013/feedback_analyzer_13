// RED CASE tests for known legacy defects.
// These tests intentionally describe the target behavior after refactoring.
// They are expected to fail against the current legacy implementation.
//
// Primary mode: Catch2.
// Fallback mode: a tiny C++17 runner is provided so this file remains readable
// in environments where Catch2 is not installed yet. Replace the fallback with
// Catch2 when the project test target is wired.

#if __has_include(<catch2/catch_test_macros.hpp>)
#include <catch2/catch_test_macros.hpp>
#define LEGACY_RED_CASES_USE_CATCH2 1
#elif __has_include(<catch2/catch.hpp>)
#include <catch2/catch.hpp>
#define LEGACY_RED_CASES_USE_CATCH2 1
#else
#define LEGACY_RED_CASES_USE_CATCH2 0
#endif

#include "../../src/cpp/Constants.h"
#include "../../src/cpp/Feedback.h"
#include "../../src/cpp/Filters.h"
#include "../../src/cpp/Logger.h"
#include "../../src/cpp/Session.h"
#include "../../src/cpp/TextAnalyzer.h"

#include <iostream>
#include <stdexcept>
#include <sstream>
#include <string>
#include <vector>

#if !LEGACY_RED_CASES_USE_CATCH2
namespace legacy_red_case_runner {

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

} // namespace legacy_red_case_runner

#define LEGACY_CONCAT_INNER(a, b) a##b
#define LEGACY_CONCAT(a, b) LEGACY_CONCAT_INNER(a, b)
#define TEST_CASE(name, tags)                                                                     \
    static void LEGACY_CONCAT(legacy_red_case_test_, __LINE__)();                                 \
    namespace {                                                                                   \
    const bool LEGACY_CONCAT(legacy_red_case_registered_, __LINE__) =                              \
        legacy_red_case_runner::add(name, &LEGACY_CONCAT(legacy_red_case_test_, __LINE__));        \
    }                                                                                             \
    static void LEGACY_CONCAT(legacy_red_case_test_, __LINE__)()
#define REQUIRE(expression)                                                                       \
    do {                                                                                          \
        if (!(expression)) {                                                                      \
            throw std::runtime_error("Requirement failed: " #expression);                         \
        }                                                                                         \
    } while (false)
#define REQUIRE_FALSE(expression) REQUIRE(!(expression))
#endif

namespace {

class CoutCapture {
public:
    CoutCapture() : oldBuffer_(std::cout.rdbuf(captured_.rdbuf())) {}

    ~CoutCapture() {
        std::cout.rdbuf(oldBuffer_);
    }

    std::string str() const {
        return captured_.str();
    }

private:
    std::ostringstream captured_;
    std::streambuf* oldBuffer_;
};

class SessionResetGuard {
public:
    SessionResetGuard() {
        Session::updateCurrentFeedbacks({});
    }

    ~SessionResetGuard() {
        Session::updateCurrentFeedbacks({});
    }
};

void initLegacyKeywordTables() {
    Constants::init();
    Filters::initFilterKeywords();
}

} // namespace

TEST_CASE("RED-04: sentiment analysis and sentiment filtering use the same rules", "[red][sentiment]") {
    initLegacyKeywordTables();

    const std::vector<Feedback> feedbacks = {
        Feedback(u8"배송이 빠르고 정확합니다")
    };

    TextAnalyzer analyzer;
    Filters filters;

    const auto sentiment = analyzer.sent(feedbacks);
    const auto positiveFiltered = filters.fil(feedbacks, u8"긍정", u8"전체");

    REQUIRE(sentiment.at(u8"긍정") == static_cast<int>(positiveFiltered.size()));
}

TEST_CASE("RED-05: category analysis and category filtering use the same category rules", "[red][category]") {
    initLegacyKeywordTables();

    const std::vector<Feedback> feedbacks = {
        Feedback(u8"제품 품질이 중요합니다")
    };

    TextAnalyzer analyzer;
    Filters filters;

    const auto keywordResults = analyzer.kw(feedbacks);
    const auto qualityFiltered = filters.fil(feedbacks, u8"전체", u8"품질");

    REQUIRE(keywordResults.at(u8"품질") == static_cast<int>(qualityFiltered.size()));
}

TEST_CASE("RED-14: session initialization isolates tests from previous feedback state", "[red][session]") {
    SessionResetGuard guard;

    Session::getCurrentFeedbacks().push_back(Feedback(u8"이전 테스트 데이터"));
    Session::initSessionStateUgly();

    REQUIRE(Session::getCurrentFeedbacks().empty());
}

TEST_CASE("RED-15: filtering does not print raw user feedback to stdout", "[red][logging][privacy]") {
    initLegacyKeywordTables();

    const std::string sensitiveFeedback = u8"고객 전화번호 010-1234-5678 포함된 민감 피드백";
    const std::vector<Feedback> feedbacks = {
        Feedback(sensitiveFeedback)
    };

    Filters filters;
    CoutCapture capture;

    (void)filters.fil(feedbacks, u8"전체", u8"전체");

    REQUIRE(capture.str().find(sensitiveFeedback) == std::string::npos);
}

TEST_CASE("RED-15: debug logging is disabled by default for operations", "[red][logging]") {
    REQUIRE_FALSE(Logger::isDebugMode());
}

#if !LEGACY_RED_CASES_USE_CATCH2
int main() {
    int failed = 0;
    for (const auto& test : legacy_red_case_runner::registry()) {
        try {
            test.fn();
            std::cout << "[PASS] " << test.name << '\n';
        } catch (const std::exception& e) {
            ++failed;
            std::cout << "[FAIL] " << test.name << " - " << e.what() << '\n';
        }
    }
    return failed == 0 ? 0 : 1;
}
#endif
