#include "Constants.h"

std::map<std::string, std::vector<std::string>> Constants::SENTIMENT_KEYWORDS;
std::map<std::string, std::map<std::string, std::vector<std::string>>> Constants::CATEGORY_KEYWORDS;

void Constants::init() {
    SENTIMENT_KEYWORDS[u8"긍정"] = {
        u8"좋아요", u8"만족", u8"감사", u8"최고", u8"좋은", u8"훌륭", u8"추천", u8"좋았어요",
        u8"좋습니다", u8"최고입니다", u8"감사합니다", u8"만족스럽", u8"좋았습니다",
        u8"최고에요", u8"기뻐요", u8"만족합니다", u8"굿", u8"최고다", u8"와우", u8"아주 좋아",
        u8"좋아요", u8"만족", u8"감사", u8"최고", u8"좋은", u8"훌륭", u8"추천", u8"좋았어요",
        u8"좋습니다", u8"최고입니다", u8"감사합니다", u8"만족스럽", u8"좋았습니다"
    };

    SENTIMENT_KEYWORDS[u8"부정"] = {
        u8"나쁘", u8"불만", u8"실망", u8"최악", u8"별로", u8"불편", u8"불만족", u8"문제",
        u8"불량", u8"불량품", u8"환불", u8"교환", u8"불만족스럽", u8"실망스럽",
        u8"짜증", u8"화남", u8"별로에요", u8"엉망", u8"최악이다", u8"실패", u8"구려",
        u8"나쁘", u8"불만", u8"실망", u8"최악", u8"별로", u8"불편", u8"불만족", u8"문제",
        u8"불량", u8"불량품", u8"환불", u8"교환", u8"불만족스럽", u8"실망스럽"
    };

    // Category keywords - 배송
    CATEGORY_KEYWORDS[u8"배송"]["main"] = {u8"배송", u8"택배", u8"배달", u8"물류", u8"배송지연", u8"배송시간", u8"퀵", u8"소포"};
    CATEGORY_KEYWORDS[u8"배송"]["time"] = {u8"배송지연", u8"배송시간", u8"퀵"};
    CATEGORY_KEYWORDS[u8"배송"]["type"] = {u8"택배", u8"배달", u8"소포"};
    CATEGORY_KEYWORDS[u8"배송"]["status"] = {u8"물류", u8"배송"};

    // 품질
    CATEGORY_KEYWORDS[u8"품질"]["main"] = {u8"품질", u8"재질", u8"내구성", u8"마감", u8"제품상태", u8"품질문제", u8"내용물", u8"고장"};
    CATEGORY_KEYWORDS[u8"품질"]["physical"] = {u8"재질", u8"내구성", u8"마감"};
    CATEGORY_KEYWORDS[u8"품질"]["state"] = {u8"제품상태", u8"품질문제", u8"고장"};
    CATEGORY_KEYWORDS[u8"품질"]["content"] = {u8"내용물"};

    // 가격
    CATEGORY_KEYWORDS[u8"가격"]["main"] = {u8"가격", u8"비용", u8"할인", u8"가성비", u8"가격대", u8"비싸", u8"저렴", u8"금액", u8"요금"};
    CATEGORY_KEYWORDS[u8"가격"]["amount"] = {u8"가격", u8"비용", u8"금액", u8"요금"};
    CATEGORY_KEYWORDS[u8"가격"]["discount"] = {u8"할인", u8"가성비", u8"가격대"};
    CATEGORY_KEYWORDS[u8"가격"]["evaluation"] = {u8"비싸", u8"저렴"};

    // 서비스
    CATEGORY_KEYWORDS[u8"서비스"]["main"] = {u8"서비스", u8"응대", u8"상담", u8"문의", u8"답변", u8"고객서비스", u8"친절", u8"불친절"};
    CATEGORY_KEYWORDS[u8"서비스"]["interaction"] = {u8"응대", u8"상담", u8"문의", u8"답변"};
    CATEGORY_KEYWORDS[u8"서비스"]["quality"] = {u8"친절", u8"불친절"};
    CATEGORY_KEYWORDS[u8"서비스"]["type_"] = {u8"서비스", u8"고객서비스"};

    // 사용성
    CATEGORY_KEYWORDS[u8"사용성"]["main"] = {u8"사용", u8"편리", u8"불편", u8"사용법", u8"설명서", u8"사용방법", u8"어렵", u8"쉽게"};
    CATEGORY_KEYWORDS[u8"사용성"]["ease"] = {u8"편리", u8"불편", u8"쉽게", u8"어렵"};
    CATEGORY_KEYWORDS[u8"사용성"]["guide"] = {u8"사용법", u8"설명서", u8"사용방법"};
    CATEGORY_KEYWORDS[u8"사용성"]["action"] = {u8"사용"};
}
