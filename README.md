# Feedback Analyzer
![feedback_analyzer](./feedback_analyzer.png)

고객 피드백 분석 시스템은 자연어 기반 고객 피드백 데이터를 수집, 분류, 시각화하는 기능을 제공하는 C++ (cpp-httplib) 기반 웹 애플리케이션입니다.

## 주요 기능

- 텍스트 피드백 입력 (수동/CSV 업로드)
- 키워드 기반 피드백 분류
- 감정 분석 (긍정/부정/중립)
- 피드백 필터링 및 검색
- 분석 결과 시각화
- 결과 CSV 다운로드

## 요구사항

- C++17 이상 지원 컴파일러 (MSVC, GCC, Clang)
- CMake 3.14 이상

## 설치 방법
저장소 클론
```
git clone [repository-url]
cd feedback_analyzer_cpp
```

## 빌드 방법
```
rmdir /q /s build
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_CXX_COMPILER=C:/mingw64/bin/g++.exe
cmake --build build
```

## 실행 방법
```
build\feedback_analyzer.exe
```

## 프로젝트 구조

```
feedback_analyzer_cpp/
├── src/cpp/
│   ├── main.cpp           # HTTP 서버 및 라우팅 (cpp-httplib 기반)
│   ├── httplib.h           # cpp-httplib 헤더 라이브러리
│   ├── Feedback.h          # 피드백 데이터 모델
│   ├── TextAnalyzer.h/cpp  # 텍스트 분석 로직
│   ├── Filters.h/cpp       # 필터링
│   ├── UIComponents.h/cpp  # UI 컴포넌트
│   ├── Session.h/cpp       # 상태 관리
│   ├── Logger.h/cpp        # 로깅
│   ├── Constants.h/cpp     # 상수 정의
│   └── FileHandler.h       # 파일 처리
├── CMakeLists.txt          # CMake 빌드 설정
├── project_purpose.md      # 프로젝트 목적 문서
└── README.md               # 프로젝트 설명
```

## 사용 방법

1. 웹 브라우저에서 `http://localhost:8080` 접속
2. 피드백 텍스트 입력 또는 CSV 파일 업로드
3. 감정/키워드 필터로 결과 필터링
4. 필요시 결과 다운로드

## CSV 파일 형식

입력 CSV 파일은 다음과 같은 형식이어야 합니다:
- 필수 컬럼: `text`
- 텍스트 컬럼에 피드백 내용 포함
