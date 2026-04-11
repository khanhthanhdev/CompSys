# FTC Tournament Manager - Important Test Cases

These are the report-level CLI checks that stay automated in
`tests/run_report_cases.sh`.
They emphasize import robustness, menu validation, and score-entry guardrails.

Screenshot summary: [fig/fig5_report_tests.svg](../fig/fig5_report_tests.svg)

| ID | Feature | Input / Fixture | Expected Console Output | Status |
|---|---|---|---|---|
| TC-01 | Import Teams | `1 -> 7` with normal `data/teams.csv` | `Loaded 10 teams.` | PASS |
| TC-02 | Import Teams Missing File | `1 -> 7` with `data/teams.csv` absent | `Warning: Cannot open data/teams.csv` and `Failed to load teams. Keeping existing data.` | PASS |
| TC-03 | Import Teams Malformed Row | `1 -> 7` with malformed team row like `bad,row` | `Warning: Skipping malformed row: bad,row` and `Loaded 2 teams.` | PASS |
| TC-04 | Import Schedule | `2 -> 7` with normal `data/matches.csv` | `Loaded 20 matches (3 pending).` | PASS |
| TC-05 | Import Schedule Missing File | `2 -> 7` with `data/matches.csv` absent | `Warning: Cannot open data/matches.csv` and `Failed to load matches. Keeping existing data.` | PASS |
| TC-06 | Import Schedule Malformed Row | `2 -> 7` with malformed match row like `bad,row` | `Warning: Skipping malformed row: bad,row` and `Loaded 2 matches (1 pending).` | PASS |
| TC-07 | Invalid Main Menu Input | `abc -> 7` | `Invalid input. Please enter a number.` | PASS |
| TC-08 | Play Match Without Teams | `3 -> 7` | `No teams loaded. Please import teams first (option 1).` | PASS |
| TC-09 | Play Match Without Schedule | `1 -> 3 -> 7` with no `data/matches.csv` fixture | `No matches loaded. Please import the schedule first (option 2).` | PASS |
| TC-10 | Invalid Score Input | `1 -> 2 -> 3 -> a -> 7` | `Invalid score input. Match not recorded.` | PASS |
