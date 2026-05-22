#include "Filters.h"

std::map<std::string, std::vector<std::string>> Filters::S_KEYWORDS;

void Filters::initFilterKeywords() {
    S_KEYWORDS = Constants::SENTIMENT_KEYWORDS;
    S_KEYWORDS[u8"중립"] = {
        u8"괜찮", u8"보통", u8"평범", u8"무난", u8"그냥", u8"전반적", u8"완료",
        u8"적당", u8"나쁘지 않", u8"특별", u8"없"
    };
}
