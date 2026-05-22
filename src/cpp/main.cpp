#include "httplib.h"
#include "Constants.h"
#include "Filters.h"
#include "Logger.h"
#include "RequestHandlers.h"

static std::map<std::string, std::vector<Feedback>>& fil_data = RequestHandlers::filteredResults();

int main() {
    Constants::init();
    Filters::initFilterKeywords();

    httplib::Server svr;

    svr.Get("/", RequestHandlers::handleIndex);
    svr.Post("/analyze", RequestHandlers::handleAnalyze);
    svr.Post("/upload", RequestHandlers::handleUpload);
    svr.Post("/filter", RequestHandlers::handleFilter);
    svr.Get("/download", RequestHandlers::handleDownload);

    Logger::logInfo(u8"서버가 http://localhost:8080 에서 시작됩니다.");
    if (!svr.listen("0.0.0.0", 8080)) {
        Logger::logError(u8"서버 시작 실패: 포트 8080 바인딩을 확인하세요.");
        return 1;
    }

    return 0;
}
