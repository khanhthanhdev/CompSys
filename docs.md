# Product Requirements Document
## FTC Tournament Management System
**Language:** C  
**Type:** Console-based Application  
**Version:** 1.0  
**Last Updated:** 2026-04-01

---

## 1. Overview

A lightweight, console-based backend system for managing a FIRST Tech Challenge (FTC)-style robotics tournament. The system handles team registration, match scheduling, score entry, and live rankings — all from a terminal interface, persisted via CSV files.

---

## 2. Goals

- Provide a minimal but complete tournament management workflow for a single competition event.
- Allow an operator to import teams, load a match schedule, enter scores match-by-match, and view up-to-date rankings at any time.
- Store all data in flat CSV files so results are human-readable and portable.
- Meet all academic requirements: structs, pointers, dynamic memory, linked list queue, file I/O, and modular build.

---

## 3. Non-Goals (Out of Scope for v1.0)

- GUI or web interface.
- Automatic schedule generation (manual CSV import only in v1.0).
- Playoff / elimination bracket.
- Network sync or multi-operator support.
- Alliance selection.

---

## 4. User Roles

| Role | Description |
|---|---|
| **Operator** | The single user running the program at the tournament table. Imports data, enters scores, views standings. |

---

## 5. Functional Requirements

### 5.1 Team Management

| ID | Requirement |
|---|---|
| F-TM-01 | Import teams from a CSV file with columns: `team_id, team_name`. |
| F-TM-02 | Store each team in a **dynamically allocated struct** in a singly linked list. |
| F-TM-03 | Display the full team roster with ID and name. |
| F-TM-04 | Persist the team list back to CSV on exit. |

**Team CSV format:**
```
team_id,team_name
1001,Circuit Breakers
1002,Gear Grinders
1003,Iron Wolves
```

---

### 5.2 Match Schedule

| ID | Requirement |
|---|---|
| F-MS-01 | Import a match schedule from a CSV file with columns: `match_num, red_team_id, blue_team_id`. |
| F-MS-02 | Each match is stored as a dynamically allocated struct in a singly linked list. |
| F-MS-03 | Matches are loaded into a **FIFO Queue** (singly linked list) of pending matches. |
| F-MS-04 | Display the full schedule showing match number, red alliance team, and blue alliance team. |
| F-MS-05 | Distinguish between `PENDING` and `PLAYED` matches in the display. |

**Match Schedule CSV format:**
```
match_num,red_team_id,blue_team_id
1,1001,1002
2,1003,1004
3,1001,1003
```

---

### 5.3 Score Entry

| ID | Requirement |
|---|---|
| F-SE-01 | Dequeue the next pending match from the queue (match 1 → last). |
| F-SE-02 | Display the current match: match number, red team name, blue team name. |
| F-SE-03 | Prompt the operator to enter the red alliance score and blue alliance score (non-negative integers). |
| F-SE-04 | Validate input: scores must be ≥ 0 and numeric. Re-prompt on invalid input. |
| F-SE-05 | On submission, mark the match as `PLAYED` and record scores. |
| F-SE-06 | Update both teams' stats immediately after score submission. |
| F-SE-07 | Append the result to `data/results.csv`. |
| F-SE-08 | If no pending matches remain, display a message: `"All matches have been played."` |

---

### 5.4 Ranking Calculation

| ID | Requirement |
|---|---|
| F-RK-01 | Ranking Points (RP) awarded per match: Win = **3 RP**, Tie = **1 RP**, Loss = **0 RP**. |
| F-RK-02 | Rankings are sorted by RP (descending). Tiebreaker: total cumulative score (descending). |
| F-RK-03 | Display rankings as a formatted table: Rank, Team ID, Team Name, RP, W, T, L, Total Score. |
| F-RK-04 | Rankings are recalculated and re-sorted every time the rankings view is requested. |
| F-RK-05 | Export current rankings to `data/rankings.csv` on demand and on exit. |

**Rankings CSV format:**
```
rank,team_id,team_name,rp,wins,ties,losses,total_score
1,1003,Iron Wolves,9,3,0,0,450
2,1001,Circuit Breakers,6,2,0,1,390
```

---

### 5.5 Results Log

| ID | Requirement |
|---|---|
| F-RL-01 | All completed match results are stored in `data/results.csv`. |
| F-RL-02 | The operator can view a history of all played matches with scores and outcomes. |

**Results CSV format:**
```
match_num,red_team_id,blue_team_id,red_score,blue_score,winner
1,1001,1002,120,95,RED
2,1003,1004,88,88,TIE
```

---

### 5.6 Main Menu

The program presents a numbered console menu on launch and after each action:

```
========================================
   FTC TOURNAMENT MANAGEMENT SYSTEM
========================================
 [1] Import Teams from CSV
 [2] Import Match Schedule from CSV
 [3] Play Next Match (Enter Scores)
 [4] View Match Schedule
 [5] View Match Results
 [6] View Rankings
 [7] Export Rankings to CSV
 [8] Exit
========================================
Select option:
```

---

## 6. Non-Functional Requirements

| ID | Requirement |
|---|---|
| NF-01 | All dynamic data (teams, matches, queue nodes) must use `malloc` or `calloc`. No static arrays for the main database. |
| NF-02 | All allocated memory must be freed before program exit. |
| NF-03 | The program must not crash on malformed CSV input — skip bad rows and print a warning. |
| NF-04 | Console output must be clearly formatted with aligned columns and separator lines. |
| NF-05 | Code must compile with `gcc -Wall -Wextra` with no errors. |

---

## 7. Technical Architecture

### 7.1 Module Structure

```
ftc_tournament/
├── Makefile
├── include/
│   ├── tournament.h      # Shared types, structs, constants
│   ├── team.h            # Team functions
│   ├── match.h           # Match functions
│   ├── queue.h           # Match queue (linked list)
│   ├── ranking.h         # Ranking sort & display
│   └── fileio.h          # CSV read/write utilities
├── src/
│   ├── main.c            # Entry point, main menu loop
│   ├── team.c            # Team CRUD, stats update
│   ├── match.c           # Match list management
│   ├── queue.c           # FIFO queue (enqueue/dequeue)
│   ├── ranking.c         # Ranking calculation & sort
│   └── fileio.c          # CSV import/export
└── data/
    ├── teams.csv
    ├── matches.csv
    ├── results.csv
    └── rankings.csv
```

### 7.2 Core Data Structures

**Team** (singly linked list node):
```c
typedef struct Team {
    int  id;
    char name[64];
    int  ranking_points;
    int  matches_played;
    int  wins, ties, losses;
    int  total_score;
    struct Team *next;
} Team;
```

**Match** (singly linked list node):
```c
typedef struct Match {
    int         match_num;
    int         red_team_id;
    int         blue_team_id;
    int         red_score;
    int         blue_score;
    MatchStatus status;   /* PENDING | PLAYED */
    struct Match *next;
} Match;
```

**Match Queue** (FIFO, wraps Match pointers):
```c
typedef struct QueueNode {
    Match            *match;
    struct QueueNode *next;
} QueueNode;

typedef struct {
    QueueNode *front;
    QueueNode *rear;
    int        size;
} MatchQueue;
```

### 7.3 Key Algorithms

**Score submission flow:**
1. Dequeue front node from `MatchQueue`.
2. Read scores from stdin, validate ≥ 0.
3. Determine outcome: red win / blue win / tie.
4. Award RP to each team via `team_update_stats()`.
5. Mark `match->status = MATCH_PLAYED`.
6. Append row to `results.csv`.

**Ranking sort:**
- Copy team linked list pointers into a temporary dynamic array.
- Sort with `qsort()` using a comparator: primary key = RP descending, secondary key = total score descending.
- Print sorted array; free temporary array.

---

## 8. File I/O Specification

| File | Read | Write | Append |
|---|---|---|---|
| `data/teams.csv` | On import (F-TM-01) | On exit | — |
| `data/matches.csv` | On import (F-MS-01) | On exit | — |
| `data/results.csv` | On "View Results" | — | After each match (F-SE-07) |
| `data/rankings.csv` | — | On export / exit | — |

All files use comma-separated values with a header row. The program skips the header on read and writes it fresh on write.

---

## 9. Constraints & Academic Requirements Mapping

| Academic Requirement | Implementation |
|---|---|
| ≥ 300 lines of code | ~600+ lines across 6 source files |
| ≥ 5 core functions | `team_update_stats`, `match_score_entry`, `queue_dequeue`, `ranking_sort_and_display`, `csv_import_teams`, `csv_append_result` |
| Struct + Pointers | `Team*`, `Match*`, `QueueNode*` linked list manipulation |
| Dynamic memory (`malloc`/`calloc`) | Every Team, Match, QueueNode allocated on heap |
| Singly Linked List (Queue) | `MatchQueue` — FIFO queue for pending matches |
| File I/O (read + write + update) | CSV import on load, append results, overwrite rankings |
| Multiple modules + Makefile | 5 `.c` files, 5 `.h` files, 1 `Makefile` |

---

## 10. Sample Workflow (Happy Path)

```
1. Operator launches ./tournament
2. Selects [1] → imports teams.csv (5 teams loaded)
3. Selects [2] → imports matches.csv (10 matches queued)
4. Selects [4] → views full schedule (all PENDING)
5. Selects [3] → Match 1: Circuit Breakers (RED) vs Gear Grinders (BLUE)
                  Enter RED score: 145
                  Enter BLUE score: 110
                  >> RED WINS! +3 RP to Circuit Breakers
6. Selects [6] → Rankings updated and displayed
7. Repeat [3] until all matches played
8. Selects [7] → Exports final rankings.csv
9. Selects [8] → Exit, all data saved
```

---

## 11. Future Enhancements (v2.0+)

- **Auto Schedule Generation** — generate round-robin schedule from team list.
- **Playoff Bracket** — top-N teams advance to single-elimination rounds.
- **Alliance Selection** — captain/pick system post-qualification.
- **Inspector Checklist** — per-team inspection pass/fail status.
- **Color-coded terminal output** — ANSI colors for RED/BLUE alliances.