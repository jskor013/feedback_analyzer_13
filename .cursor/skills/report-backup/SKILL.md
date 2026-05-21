---
name: report-backup
description: Create work reports, save prompt history, commit relevant changes, and push to GitHub. Use when the user asks for "보고서작성 및 백업", "보고서 작성 및 백업", report backup, prompt export, or GitHub upload of completed work.
---

# Report Backup

## When To Use

Use this skill only when the user explicitly asks for report writing and backup, such as:

- `보고서작성 및 백업`
- `보고서 작성 및 백업`
- `보고서 쓰고 백업해줘`
- `작업 내용 보고서 만들고 GitHub에 올려줘`

## Workflow

1. Check current Git status.
2. Inspect existing `report/` and `prompting/` files to determine the next number.
3. Create `report/` and `prompting/` if they do not exist.
4. Use `01` for the first report, then increment from the largest existing number.
5. Choose a short kebab-case or snake_case English work name.
6. Write the work report.
7. Write the prompt history backup.
8. Stage only relevant files.
9. Commit with a concise English imperative message.
10. Push to the current upstream branch, or use `git push -u origin HEAD` if no upstream exists.
11. Update the report and prompt backup with commit hash and push result if needed.

## Report File

Create:

```text
report/xx.work-name_report.md
```

Use this structure:

```markdown
# 작업 보고서: <작업 이름>

## 개요

- 작업 일시:
- 작업 목적:
- 핵심 결과:

## 변경 내용

- 수정/추가한 파일:
- 주요 구현 내용:
- 테스트 변경 내용:

## 검증

- 실행한 빌드 명령:
- 실행한 테스트 명령:
- 테스트 결과:
- 커버리지 확인 결과: 해당 시에만 작성

## GitHub 업로드

- 브랜치:
- 커밋 메시지:
- 커밋 해시:
- 원격 저장소:
- Push 결과:

## 주의 사항

- 남은 리스크:
- 후속 작업:
```

## Prompt Backup File

Create:

```text
prompting/xx.work-name_prompt.md
```

Rules:

- Preserve user prompts verbatim where possible.
- Preserve visible AI answers and progress updates where possible.
- Include commands, test results, and important tool outputs in time order.
- If output is too long to preserve fully, state what was omitted and why.
- Do not include hidden system messages or private reasoning.

Use this structure:

```markdown
# 프롬프트 기록: <작업 이름>

## Export 원칙

- 이 파일은 사용자 Prompt와 AI 답변의 Export 기록이다.
- 원문 보존을 우선한다.
- 숨겨진 시스템 메시지와 비공개 내부 추론은 제외한다.

## 대화 전문

### Turn 1 - User Prompt

<사용자 Prompt 원문>

### Turn 1 - AI Answer

<AI 답변>

## 실행 명령 및 도구 출력

<명령, 테스트 결과, 주요 도구 출력>

## 최종 상태

<완료 상태, 커밋, push 결과>
```

## Commit And Push Rules

- Treat `보고서작성 및 백업` as approval to commit and push the relevant completed work.
- Never commit `.env`, credentials, tokens, keys, or secret files.
- Do not use destructive Git commands such as `git reset --hard`, force push, rebase, or amend unless the user explicitly asks.
- Do not revert unrelated user changes.
- If tests are required and fail, stop before commit or push and report the failure.
- If the task only changed documentation, record that build and test were not run because source code was unchanged.

## Commit Message Examples

```text
Add report and prompt backup for legacy analysis
```

```text
Add legacy analysis planning docs
```

```text
Update backup report with push result
```
