# Refactoring 작업 지시서

## 1. 목적

이 문서는 현재까지 진행된 RED 단계 개선, Golden Master 회귀테스트 구현, Phase 4 운영 안정성 요구사항을 기준으로 다음 리팩토링 작업 범위를 정의한다.

이번 리팩토링의 목적은 이미 Green으로 전환된 기능 동작을 유지하면서 코드 스멜을 줄이고 SOLID 원칙에 맞는 책임 경계를 만드는 것이다. 기능 추가가 아니라 구조 개선이므로 각 작업은 1커밋 단위로 작게 진행한다.

## 2. 참조 문서

- `docs/PRD.md`
- `README.md`
- `docs/Regacy_Code_리팩토링_테스트_계획.md`
- `docs/red_stage_development_report.md`
- `docs/회귀테스트 구현 결과 보고서.md`

## 3. 현재 상태 요약

### 3.1 이미 해결된 범위

다음 항목은 RED 단계 개선 보고서와 Golden Master 회귀테스트 결과 기준으로 이미 Green 상태다.

- 필터 결과 없음 이후 이전 다운로드 결과가 재사용되는 문제
- 필터 전 다운로드가 과거 데이터를 내려주는 문제
- 사용자 세션 간 피드백 또는 다운로드 상태가 섞이는 문제
- 감정 분석과 감정 필터 기준 불일치
- 카테고리 분석과 카테고리 필터 기준 불일치
- CSV 헤더 없음, `text` 컬럼 위치 변경, 쉼표, 따옴표, 줄바꿈 포함 데이터 처리
- 공백 입력, 파일 미선택, 빈 CSV 등 입력 실패 메시지 구분
- 서버 시작 실패 시 실패 종료와 ERROR 로그 기록
- 기본 INFO 로그에서 사용자 입력 원문 노출 방지
- Golden Master 회귀테스트 18개 Green

### 3.2 이번 리팩토링의 전제

- 사용자 기능 동작은 변경하지 않는다.
- Golden Master 18개는 모든 커밋 후 통과해야 한다.
- RED 단계에서 Green으로 전환된 케이스는 다시 실패하면 안 된다.
- 동작 변경 커밋과 구조 정리 커밋을 섞지 않는다.
- 각 커밋은 한 가지 책임 경계만 개선한다.

## 4. Refactoring 대상 Listup

| ID | 리팩토링 대상 | 현재 코드 스멜 | 주요 SOLID 관점 | 우선순위 |
| --- | --- | --- | --- | --- |
| R1 | 회귀 테스트 실행 경로 정리 | RED 테스트 실행이 CTest 표준 흐름에 완전히 편입되지 않음 | 리팩토링 안전망 | 높음 |
| R2 | `main.cpp` 요청 처리 흐름 분리 | God Function, 긴 라우트 람다, 다중 책임 | SRP, DIP | 높음 |
| R3 | 세션과 다운로드 상태 저장소 명확화 | static 전역 상태, 분산된 상태 소유권, 의미가 어색한 API | SRP, ISP | 높음 |
| R4 | 분석/필터 도메인 규칙 단일화 | 유사한 분류 로직과 `containsAny` 중복 | SRP, OCP | 높음 |
| R5 | CSV Import/Export 서비스화 | HTTP 핸들러에 CSV 정책과 검증 규칙 잔존 | SRP, OCP | 중간 |
| R6 | 운영 로그 정책 객체화 | static 출력 고정, 로그 이벤트 계약 표현 부족 | SRP, DIP | 중간 |
| R7 | 렌더링 ViewModel 분리 | HTML 생성, 메시지, 분석 표시 조건, timestamp 결합 | SRP, OCP | 중간 |
| R8 | 네이밍과 레거시 API 정리 | `initSessionStateUgly`, `getOldDataFromSession`, `sent`, `kw`, `fil` 등 의도 불명확 | ISP, 유지보수성 | 낮음 |

## 5. 1커밋 단위 Refactoring 계획

### R1. RED 회귀 테스트 실행 경로 정리

목표:

- RED 단계에서 Green으로 전환된 테스트를 일반 회귀 테스트 실행 흐름에 포함한다.
- 이후 구조 리팩토링이 기능 회귀를 만들면 즉시 감지되도록 한다.

대상:

- `CMakeLists.txt`
- `tests/red_cases/legacy_red_cases_test.cpp`
- `tests/red_cases/legacy_unautomated_red_cases_test.cpp`

작업 지시:

- 기존 RED 테스트 파일의 기대값은 변경하지 않는다.
- 수동 `g++` 실행으로 검증하던 RED 테스트를 CMake/CTest 타깃으로 등록한다.
- `golden_master`와 RED 회귀 테스트를 `ctest --test-dir build --output-on-failure` 한 번으로 실행할 수 있게 한다.
- Catch2 fallback 방식과 충돌하지 않게 별도 executable 또는 test name을 둔다.

검증:

- `cmake -S . -B build`
- `cmake --build build`
- `ctest --test-dir build --output-on-failure`
- Golden Master 18개와 RED 회귀 테스트가 모두 PASS여야 한다.

권장 커밋 메시지:

- `test: register red regression tests in ctest`

### R2. HTTP 요청 처리 흐름을 Control 계층으로 분리

목표:

- `main.cpp`의 라우트 람다가 입력 파싱, 상태 변경, 분석 호출, 렌더링, 로깅을 모두 담당하는 구조를 줄인다.
- HTTP Boundary와 애플리케이션 Control 흐름을 분리한다.

대상:

- `src/cpp/main.cpp`
- 신규 후보: `RequestHandlers.h/.cpp`, `FeedbackController.h/.cpp`

작업 지시:

- `/analyze`, `/upload`, `/filter`, `/download`의 핵심 처리 흐름을 별도 Control 함수 또는 클래스로 이동한다.
- `main.cpp`는 서버 생성, 라우트 등록, 서버 시작 결과 처리에 집중하게 한다.
- 라우트 람다는 요청/응답 객체를 Control 계층에 위임하는 얇은 Adapter 역할만 수행한다.
- 사용자에게 반환되는 HTML, 메시지, HTTP content type은 변경하지 않는다.

검증:

- GM-01, GM-02, GM-05, GM-11, GM-12, GM-15, GM-17을 우선 확인한다.
- 전체 Golden Master와 RED 회귀 테스트를 실행한다.

권장 커밋 메시지:

- `refactor: separate request handling from server setup`

### R3. 세션 상태와 다운로드 상태 저장소 명확화

목표:

- `Session`의 static 전역 상태와 `main.cpp`의 `fil_data` 분산 상태를 정리한다.
- 피드백 목록, 필터 결과, 다운로드 가능 상태의 소유자를 명확히 한다.

대상:

- `src/cpp/Session.h`
- `src/cpp/Session.cpp`
- `src/cpp/main.cpp`
- 신규 후보: `FeedbackStore.h/.cpp`, `FilteredResultStore.h/.cpp`

작업 지시:

- 피드백 상태와 필터 다운로드 상태를 같은 세션 ID 기준으로 관리하되 책임을 명확히 나눈다.
- `fil_data` 같은 전역 map을 Control 또는 Store 객체 내부로 이동한다.
- `default` 세션 처리 규칙을 명시적으로 보존한다.
- `initSessionStateUgly()`처럼 테스트 초기화를 위해 노출된 API는 후속 R8에서 이름을 바꿀 수 있도록 먼저 호출 지점을 최소화한다.

검증:

- GM-03, GM-04, GM-11, GM-12, GM-13을 반복 실행한다.
- RED-01, RED-02, RED-03, RED-14가 계속 PASS여야 한다.
- 서로 다른 `sid` 요청 간 피드백과 다운로드 결과가 섞이지 않아야 한다.

권장 커밋 메시지:

- `refactor: clarify session and filtered result storage`

### R4. 분석과 필터 도메인 규칙 단일화

목표:

- `TextAnalyzer`와 `Filters`가 유사한 감정/카테고리 판단 규칙을 각자 보유하는 중복을 제거한다.
- 신규 감정 또는 카테고리 규칙 추가 시 한 곳만 확장하도록 만든다.

대상:

- `src/cpp/TextAnalyzer.h`
- `src/cpp/TextAnalyzer.cpp`
- `src/cpp/Filters.h`
- `src/cpp/Filters.cpp`
- `src/cpp/Constants.h`
- `src/cpp/Constants.cpp`
- 신규 후보: `ClassificationRules.h/.cpp`, `FeedbackClassifier.h/.cpp`

작업 지시:

- 감정 분류와 카테고리 분류를 순수 도메인 함수로 추출한다.
- `containsAny` 중복을 제거하고 공통 규칙 객체 또는 유틸리티로 이동한다.
- `TextAnalyzer`는 집계 책임, `Filters`는 조건 적용 책임만 갖도록 분리한다.
- `Constants`는 데이터 보관에 집중하고 판단 로직을 갖지 않게 한다.

검증:

- GM-09, GM-10을 우선 확인한다.
- RED-04, RED-05가 계속 PASS여야 한다.
- 긍정, 부정, 중립, 배송, 품질, 가격, 서비스, 사용성 대표 데이터로 단위 테스트를 보강한다.

권장 커밋 메시지:

- `refactor: centralize feedback classification rules`

### R5. CSV Import/Export 서비스화

목표:

- CSV 파싱 함수는 분리되었지만 업로드 핸들러에 남아 있는 헤더 판정, `text` 컬럼 선택, 유효 row 판정 책임을 분리한다.
- CSV 입출력 정책 변경이 HTTP 요청 처리 코드를 흔들지 않게 한다.

대상:

- `src/cpp/FileHandler.h`
- `src/cpp/main.cpp`
- 신규 후보: `CsvFeedbackImporter.h/.cpp`, `CsvFeedbackExporter.h/.cpp`

작업 지시:

- CSV content를 받아 `Feedback` 목록과 실패 사유를 반환하는 Import 결과 타입을 만든다.
- 다운로드 CSV 생성은 필터 결과 목록을 받아 CSV 문자열을 반환하는 Export 함수로 분리한다.
- 빈 파일, 유효 row 없음, `text` 컬럼 없음, quoted field 처리 결과를 호출부가 구분할 수 있게 한다.
- 기존 사용자 메시지는 우선 보존하고, 상세 실패 사유 노출은 별도 요구사항 없이는 확장하지 않는다.

검증:

- GM-05, GM-06, GM-07, GM-08, GM-14, GM-15를 우선 확인한다.
- RED-06, RED-07, RED-08, RED-09, RED-10, RED-12가 계속 PASS여야 한다.

권장 커밋 메시지:

- `refactor: isolate csv import and export services`

### R6. 운영 로그 정책 객체화

목표:

- `Logger`가 static stdout/stderr 출력에 고정된 구조를 개선한다.
- PRD의 로그 계약인 level, event, count, error_reason, request_scope를 표현할 수 있는 기반을 만든다.

대상:

- `src/cpp/Logger.h`
- `src/cpp/Logger.cpp`
- `src/cpp/main.cpp`
- 신규 후보: `LogEvent.h`, `LogSink.h`, `ConsoleLogSink.h`

작업 지시:

- 기존 `logInfo`, `logWarning`, `logError`, `logDebug` 호출 결과의 출력 문구와 레벨은 보존한다.
- 내부적으로는 로그 이벤트 구조체를 만들고 Console Sink로 출력하도록 단계적으로 분리한다.
- DEBUG 기본 비활성화 정책과 사용자 원문 미노출 정책을 유지한다.
- request scope는 현재 세션 ID를 기반으로 선택적으로 기록할 수 있게 하되 기존 테스트 기대 출력을 깨뜨리지 않는다.

검증:

- GM-17, GM-18을 우선 확인한다.
- 서버 시작 실패 시 ERROR 로그가 남고 종료 코드가 1이어야 한다.
- 기본 INFO/DEBUG 로그에 사용자 입력 원문 전체가 남지 않아야 한다.

권장 커밋 메시지:

- `refactor: introduce structured logging boundary`

### R7. 렌더링 ViewModel 분리

목표:

- `renderPage()`가 HTML 문자열, 사용자 메시지, 분석 결과 표시 조건, timestamp를 함께 처리하는 구조를 줄인다.
- 화면 표현 변경과 요청 처리 변경의 영향 범위를 분리한다.

대상:

- `src/cpp/main.cpp`
- `src/cpp/UIComponents.h`
- `src/cpp/UIComponents.cpp`
- 신규 후보: `FeedbackPageViewModel.h`, `FeedbackPageRenderer.h/.cpp`

작업 지시:

- success, warning, error, 분석 결과, 표시할 피드백 목록을 ViewModel로 묶는다.
- HTML 문자열 생성은 Renderer에 모은다.
- Controller는 ViewModel만 만들고 HTML 세부 구조를 알지 않게 한다.
- 기존 화면 텍스트, form action, field name, download link는 변경하지 않는다.

검증:

- GM-01, GM-15, GM-16을 우선 확인한다.
- HTML 입력이 실행 가능한 markup으로 반영되지 않아야 한다.
- 주요 form과 select option이 기존처럼 표시되어야 한다.

권장 커밋 메시지:

- `refactor: separate page rendering from control flow`

### R8. 네이밍과 레거시 API 정리

목표:

- 기능 개선 과정에서 남은 임시 이름과 의미가 축약된 API를 정리한다.
- 개발자가 함수 이름만 보고 책임을 이해할 수 있게 한다.

대상:

- `src/cpp/Session.h`
- `src/cpp/TextAnalyzer.h`
- `src/cpp/Filters.h`
- 관련 호출부

작업 지시:

- `initSessionStateUgly()`는 테스트/앱 초기화 의도를 드러내는 이름으로 변경한다.
- `getOldDataFromSession()`은 실제 반환 의미에 맞게 변경하거나 제거한다.
- `sent`, `kw`, `fil`은 `analyzeSentiment`, `analyzeCategories`, `filterFeedbacks`처럼 책임이 드러나는 이름으로 변경한다.
- 이름 변경만 수행하고 로직 변경은 하지 않는다.

검증:

- 전체 빌드가 성공해야 한다.
- Golden Master와 RED 회귀 테스트가 모두 PASS여야 한다.
- 이름 변경 외 동작 diff가 없어야 한다.

권장 커밋 메시지:

- `refactor: clarify legacy api names`

## 6. 권장 작업 순서

1. R1로 테스트 실행 안전망을 먼저 고정한다.
2. R2와 R3로 요청 처리와 상태 소유권을 분리한다.
3. R4와 R5로 도메인 규칙과 CSV 정책을 독립 단위로 만든다.
4. R6과 R7로 운영 로그와 렌더링 Boundary를 분리한다.
5. R8로 이름과 레거시 API를 정리한다.

## 7. 커밋별 공통 완료 기준

각 커밋 전:

- 이번 커밋이 하나의 책임 경계만 다루는지 확인한다.
- 기능 변경과 단순 구조 변경이 섞이지 않았는지 확인한다.
- 변경 전 테스트 기준을 기록한다.

각 커밋 후:

- `cmake --build build`가 성공해야 한다.
- `ctest --test-dir build --output-on-failure`가 성공해야 한다.
- Golden Master 18개가 모두 PASS여야 한다.
- 해당 R 단계와 관련된 RED 회귀 케이스가 PASS여야 한다.
- 사용자 입력 원문이 기본 로그에 노출되지 않아야 한다.
- 서버 시작 실패, CSV 처리, 다운로드, 필터링의 기존 동작이 유지되어야 한다.

## 8. 리팩토링 금지 사항

- 감정 분석 알고리즘의 판단 결과를 의도적으로 변경하지 않는다.
- 카테고리 목록, 필터 option, form field name을 변경하지 않는다.
- CSV 다운로드 파일명과 기본 CSV header를 변경하지 않는다.
- 기존 사용자 메시지를 불필요하게 바꾸지 않는다.
- 테스트 기대값을 리팩토링 편의를 위해 수정하지 않는다.
- 동작 보존이 불확실한 상태에서 여러 책임을 한 커밋에 함께 수정하지 않는다.

## 9. 최종 목표 상태

리팩토링 완료 후 목표 구조는 다음과 같다.

- `main.cpp`는 서버 시작과 라우트 등록 중심의 얇은 진입점이다.
- 요청 처리, 상태 저장, 분석/필터, CSV 입출력, 렌더링, 로깅 책임이 분리되어 있다.
- 도메인 규칙은 한 곳에서 정의되고 분석과 필터가 같은 규칙을 사용한다.
- 세션별 피드백과 필터 다운로드 상태의 소유권이 명확하다.
- 로그 정책은 운영 이벤트와 출력 방식을 분리한다.
- Golden Master와 RED 회귀 테스트가 한 명령으로 실행된다.
