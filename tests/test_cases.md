# FTC Tournament Manager Test Cases

This document groups test cases for the console app into three buckets:
- Input tests
- Logic/unit-level tests
- Integration tests

The cases are written so they can be executed manually now and later converted into automated tests.

## Assumptions

- `data/teams.csv`, `data/matches.csv`, and `data/results.csv` exist in the repository.
- The executable is built with `make`.
- The program runs as a CLI and reads from standard input.
- A "played" match means `status == MATCH_PLAYED`.

## Input Tests

| ID | Scenario | Preconditions | Steps | Expected Result |
|---|---|---|---|---|
| IN-01 | Non-numeric main menu input | Program at main menu | Enter `abc` | Program prints an invalid input message and returns to the menu without crashing |
| IN-02 | Blank line at main menu | Program at main menu | Press Enter without a number | Program prints an invalid input message and returns to the menu |
| IN-03 | Out-of-range menu choice | Program at main menu | Enter `99` | Program prints `Invalid option.` and returns to the menu |
| IN-04 | Play match before loading teams | No data imported | Select option `3` | Program warns that teams must be loaded first |
| IN-05 | Play match before loading matches | Teams imported, matches not imported | Select option `3` | Program warns that the schedule must be loaded first |
| IN-06 | Non-numeric red score | Teams and matches imported | Select option `3`, then enter `a` for red score | Program rejects the input, does not record the match, and leaves it pending |
| IN-07 | Non-numeric blue score | Teams and matches imported | Select option `3`, enter a valid red score, then `b` for blue score | Program rejects the input, does not record the match, and leaves it pending |
| IN-08 | Re-import teams from missing file | Temporarily rename `data/teams.csv` | Select option `1` | Program reports the load failure and keeps existing in-memory data |
| IN-09 | Re-import matches from missing file | Temporarily rename `data/matches.csv` | Select option `2` | Program reports the load failure and keeps existing in-memory data |

## Logic / Unit-Level Tests

| ID | Scenario | Target Function(s) | Steps | Expected Result |
|---|---|---|---|---|
| LG-01 | Team stats update for win | `team_update_stats` | Call with `my_score > opp_score` | Wins increments by 1, ranking points increase by 3, total score adds `my_score` |
| LG-02 | Team stats update for tie | `team_update_stats` | Call with `my_score == opp_score` | Ties increments by 1, ranking points increase by 1, total score adds `my_score` |
| LG-03 | Team stats update for loss | `team_update_stats` | Call with `my_score < opp_score` | Losses increments by 1, ranking points do not increase, total score adds `my_score` |
| LG-04 | Reset all team stats | `team_reset_stats` | Build a list with non-zero stats and call reset | All ranking, match, win, tie, loss, and score fields become zero |
| LG-05 | Rebuild from played matches only | `team_rebuild_stats_from_matches` | Provide a team list and mixed played/pending matches | Only played matches affect team totals; pending matches are ignored |
| LG-06 | Ranking sort by RP | `ranking_sort` | Provide teams with different ranking points | Higher RP ranks first |
| LG-07 | Ranking sort tie-break by total score | `ranking_sort` | Provide teams with equal RP but different total scores | Higher total score ranks first |
| LG-08 | Ranking sort tie-break by team ID | `ranking_sort` | Provide teams with equal RP and total score | Lower team ID ranks first |
| LG-09 | Build pending queue | `queue_build_pending` | Provide a mixed match list | Only pending matches are enqueued, in the same order as the source list |
| LG-10 | Dequeue order | `queue_dequeue` | Build a queue of pending matches | Matches come out in FIFO order and queue size decreases correctly |
| LG-11 | Score entry idempotence | `match_enter_score` | Call on a match that is already played | Function returns without double-counting stats |
| LG-12 | CSV load with header row | `file_load_teams` and `file_load_matches` | Load files that begin with headers | Header row is skipped and valid rows are loaded |

## Integration Tests

| ID | Scenario | Steps | Expected Result |
|---|---|---|---|
| IT-01 | Team import then match import | Run `1 -> 2 -> 5 -> 7` | Rankings match the replayed match data from `matches.csv` |
| IT-02 | Match import then team import | Run `2 -> 1 -> 5 -> 7` | Rankings are identical to IT-01, proving import order does not matter |
| IT-03 | Play next match updates standings | Run `1 -> 2 -> 3`, enter valid scores, then `5` | The played match becomes `PLAYED` and the affected team stats change immediately |
| IT-04 | Invalid score does not consume queue item | Run `1 -> 2 -> 3`, enter invalid score input, then `4` | The current match remains pending and is still shown in the schedule |
| IT-05 | Re-import teams does not leak or duplicate data | Run `1 -> 1 -> 5 -> 7` | The second load replaces the first team list cleanly and rankings still work |
| IT-06 | Re-import matches does not leak or duplicate queue state | Run `1 -> 2 -> 2 -> 7` | The second load rebuilds the queue cleanly and pending count stays correct |
| IT-07 | Save on exit persists team and match state | Run `1 -> 2 -> 3 -> 7`, then inspect CSV output files | `teams.csv` and `matches.csv` reflect the played match state after exit |
| IT-08 | Export rankings writes sorted standings | Run `1 -> 2 -> 5 -> 6 -> 7` | `data/rankings.csv` is created or overwritten with the current sorted standings |
| IT-09 | View schedule shows played and pending matches | Run `1 -> 2 -> 4 -> 7` | Played matches show scores and results; pending matches show `PENDING` |
| IT-10 | Missing input files do not crash the CLI | Temporarily remove one or more CSV files, then run the corresponding import action | Program prints a warning and continues running |

## Notes For Automation

- Input tests are good candidates for a shell-based CLI harness.
- Logic tests are good candidates for a small C unit-test binary that links the existing source files.
- Integration tests should run against temporary copies of the CSV files so the repository data does not get mutated.

