# FTC Tournament Management System (RMS)

A console-based ranking management system for FIRST Tech Challenge (FTC) robotics tournaments.

---

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Project Structure](#project-structure)
- [Data Structures](#data-structures)
- [Workflow Diagram](#workflow-diagram)
- [Menu Options](#menu-options)
- [File Formats](#file-formats)
- [Build & Run](#build--run)
- [Usage Guide](#usage-guide)
- [Ranking System](#ranking-system)

---

## Overview

The RMS (Ranking Management System) is a lightweight C application that manages:
- Team registration and data
- Match scheduling and queue management
- Score entry and results tracking
- Live rankings calculation and export

All data is persisted in CSV files for easy viewing and portability.

---

## Features

- **Team Management** - Import teams from CSV, store in linked list
- **Match Schedule** - Load matches, track pending/played status
- **Score Entry** - Play matches sequentially, record scores
- **Rankings** - Auto-calculate standings by Ranking Points
- **Export** - Save rankings to CSV for distribution

---

## Project Structure

```
rms_project/
├── Makefile              # Build configuration
├── include/
│   └── tournament.h      # Header: structs, enums, function declarations
├── src/
│   ├── main.c            # Entry point, main menu loop
│   ├── team.c            # Team creation, search, stats update
│   ├── match.c           # Match management, score entry, display
│   ├── queue.c           # FIFO queue for pending matches
│   ├── ranking.c         # Sorting and display logic
│   └── file_io.c         # CSV read/write operations
└── data/
    ├── teams.csv         # Team database
    ├── matches.csv       # Match schedule
    ├── results.csv       # Completed match results
    └── rankings.csv      # Exported standings
```

---

## Data Structures

### Team (Singly Linked List)
```c
typedef struct Team {
    int    id;
    char   name[MAX_NAME_LEN];
    int    ranking_points;
    int    matches_played;
    int    wins, ties, losses;
    int    total_score;
    struct Team *next;
} Team;
```

### Match (Singly Linked List)
```c
typedef struct Match {
    int         match_num;
    int         red_team_id;
    int         blue_team_id;
    int         red_score;
    int         blue_score;
    MatchStatus status;   // PENDING or PLAYED
    struct Match *next;
} Match;
```

### Match Queue (FIFO)
```c
typedef struct {
    QueueNode *front;
    QueueNode *rear;
    int        size;
} MatchQueue;
```

---

## Workflow Diagram

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           FTC TOURNAMENT SYSTEM                             │
└─────────────────────────────────────────────────────────────────────────────┘
                                      │
                                      ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                              MAIN MENU LOOP                                 │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  [1] Import Teams    │  [2] Import Schedule  │  [3] Play Match     │   │
│  │  [4] View Schedule   │  [5] View Rankings    │  [6] Export CSV     │   │
│  │  [7] Exit                                                               │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────────────────┘
         │                    │                    │
         ▼                    ▼                    ▼
┌─────────────────┐  ┌─────────────────┐  ┌─────────────────────────────┐
│  LOAD TEAMS     │  │  LOAD MATCHES   │  │      PLAY MATCH FLOW        │
│  file_io.c      │  │  file_io.c      │  │                             │
│                 │  │                 │  │  1. Dequeue next match      │
│  Read CSV ──────┤  │  Read CSV ──────┤  │  2. Display teams           │
│  Parse rows     │  │  Parse rows     │  │  3. Enter scores            │
│  Create Team*   │  │  Create Match*  │  │  4. Determine winner        │
│  Link list      │  │  Build queue    │  │  5. Update team stats       │
│                 │  │                 │  │  6. Append to results.csv   │
└─────────────────┘  └─────────────────┘  └─────────────────────────────┘
         │                    │                    │
         │                    │                    ▼
         │                    │         ┌─────────────────────────────┐
         │                    │         │      UPDATE TEAM STATS      │
         │                    │         │                             │
         │                    │         │  For each team:             │
         │                    │         │  - Add RP (3/1/0)           │
         │                    │         │  - Update W-T-L             │
         │                    │         │  - Add to total_score       │
         │                    │         └─────────────────────────────┘
         │                    │                    │
         ▼                    ▼                    ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                           DATA PERSISTENCE                                  │
│                                                                             │
│   ┌──────────────┐  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐  │
│   │  teams.csv   │  │ matches.csv  │  │ results.csv  │  │ rankings.csv │  │
│   │  (Read/Write)│  │ (Read/Write) │  │  (Append)    │  │   (Write)    │  │
│   └──────────────┘  └──────────────┘  └──────────────┘  └──────────────┘  │
└─────────────────────────────────────────────────────────────────────────────┘
                                      │
                                      ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                              VIEW & EXPORT                                  │
│                                                                             │
│   ┌─────────────────────┐           ┌─────────────────────────────────┐    │
│   │  VIEW SCHEDULE      │           │      VIEW RANKINGS              │    │
│   │  ─────────────      │           │      ────────────               │    │
│   │  Match | Red | Blue │           │      Rank | Team | RP | W-T-L   │    │
│   │    1   | 101 | 102  │           │      ─────────────────────────  │    │
│   │   Score | Status    │           │      Sort by:                   │    │
│   │   ? - ? | PENDING   │           │      1. Ranking Points (desc)   │    │
│   │  45-32  | PLAYED    │           │      2. Total Score (desc)      │    │
│   └─────────────────────┘           └─────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## Menu Options

| Option | Action | Description |
|--------|--------|-------------|
| [1] | Import Teams | Load teams from `data/teams.csv` |
| [2] | Import Schedule | Load matches from `data/matches.csv` |
| [3] | Play Next Match | Dequeue match, enter scores, update stats |
| [4] | View Schedule & Results | Show all matches with status |
| [5] | View Rankings | Display sorted standings table |
| [6] | Export Rankings | Save rankings to `data/rankings.csv` |
| [7] | Exit | Save data and quit |

---

## File Formats

### teams.csv
```csv
team_id,team_name,rp,wins,ties,losses,total_score
1001,Red Dragons,9,3,0,0,312
1002,Blue Thunder,3,1,0,2,189
```

### matches.csv
```csv
match_num,red_team_id,blue_team_id,red_score,blue_score,status
1,1001,1002,100,56,1
2,1003,1001,0,0,0
```

### results.csv
```csv
match_num,red_team_id,blue_team_id,red_score,blue_score,winner
1,1001,1002,100,56,RED
```

### rankings.csv
```csv
rank,team_id,team_name,rp,wins,ties,losses,total_score
1,1001,Red Dragons,9,3,0,0,312
```

---

## Build & Run

```bash
# Navigate to project directory
cd rms_project

# Build
make

# Run
./tournament

# Clean build artifacts
make clean
```

---

## Usage Guide

### Recommended Workflow

```
┌──────────────────────────────────────────────────────────────────┐
│  1. Launch Application                                           │
│     $ ./tournament                                               │
│                          │                                       │
│                          ▼                                       │
│  2. Import Teams (Option 1)                                      │
│     - Loads teams from data/teams.csv                            │
│     - "Loaded 10 teams."                                         │
│                          │                                       │
│                          ▼                                       │
│  3. Import Match Schedule (Option 2)                             │
│     - Loads matches from data/matches.csv                        │
│     - "Loaded 20 matches (18 pending)."                          │
│                          │                                       │
│                          ▼                                       │
│  4. Play Matches (Option 3) - Repeat for each match              │
│     - Match displays: "Match 1: Red Dragons (RED) vs ..."        │
│     - Enter scores for both alliances                            │
│     - Stats updated automatically                                │
│                          │                                       │
│                          ▼                                       │
│  5. View Rankings (Option 5) - Anytime                           │
│     - See current standings                                      │
│     - Updated after each match                                   │
│                          │                                       │
│                          ▼                                       │
│  6. Export Rankings (Option 6) - When done                       │
│     - Saves to data/rankings.csv                                 │
│                          │                                       │
│                          ▼                                       │
│  7. Exit (Option 7)                                              │
│     - All data saved                                             │
└──────────────────────────────────────────────────────────────────┘
```

### Sample Session

```
$ ./tournament

Welcome to FTC Tournament Manager!

========================================
   FTC TOURNAMENT MANAGEMENT SYSTEM
========================================
 [1] Import Teams from CSV
 [2] Import Match Schedule from CSV
 [3] Play Next Match (Enter Scores)
 [4] View Match Schedule & Results
 [5] View Rankings
 [6] Export Rankings to CSV
 [7] Exit
========================================
Select option: 1
Loaded 10 teams.

Select option: 2
Loaded 20 matches (18 pending).

Select option: 3
Match 1: Red Dragons (RED) vs Blue Thunder (BLUE)
Enter RED score: 100
Enter BLUE score: 56
Score recorded.

Select option: 5

Rank   Team ID  Name                 RP    W     T    L     Total Score
============================================================================
1      1001     Red Dragons          3     1     0    0     100
2      1002     Blue Thunder         0     0     0    1     56
...
```

---

## Ranking System

### Points Calculation

| Result | Ranking Points (RP) |
|--------|---------------------|
| Win | 3 RP |
| Tie | 1 RP |
| Loss | 0 RP |

### Sort Order

1. **Primary**: Ranking Points (descending)
2. **Secondary**: Total Score (descending)
3. **Tiebreaker**: Team ID (ascending)

### Stats Tracked Per Team

- `ranking_points` - Total RP earned
- `matches_played` - Number of matches completed
- `wins`, `ties`, `losses` - Match outcomes
- `total_score` - Sum of all scores across matches

---

## Technical Highlights

| Requirement | Implementation |
|-------------|----------------|
| Dynamic Memory | `calloc` for all Team, Match, QueueNode |
| Linked Lists | Singly linked lists for teams and matches |
| Queue (FIFO) | MatchQueue with front/rear pointers |
| File I/O | CSV read, write, and append operations |
| Modular Build | Separate `.c` files compiled via Makefile |
| Memory Safety | All allocations freed before exit |

---

## Module Pseudocode

### 1. Main Menu Loop (`main.c`)

```
FUNCTION main():
    teams = NULL
    matches = NULL
    queue = empty MatchQueue
    
    WHILE true:
        PRINT menu
        choice = GET_USER_INPUT()
        
        SWITCH choice:
            CASE 1:  // Import Teams
                teams = file_load_teams(TEAMS_FILE)
                PRINT "Loaded N teams"
                
            CASE 2:  // Import Schedule
                matches = file_load_matches(MATCHES_FILE)
                queue_build_pending(&queue, matches)
                PRINT "Loaded N matches"
                
            CASE 3:  // Play Match
                IF teams IS NULL:
                    PRINT "Load teams first"
                    BREAK
                    
                match = queue_dequeue(&queue)
                IF match IS NULL:
                    PRINT "All matches played"
                ELSE:
                    red_score = PROMPT("Enter RED score")
                    blue_score = PROMPT("Enter BLUE score")
                    match_enter_score(match, teams, red_score, blue_score)
                    
            CASE 4:  // View Schedule
                match_print_all(matches, teams)
                
            CASE 5:  // View Rankings
                sorted = ranking_sort(teams)
                ranking_display(sorted)
                FREE sorted array
                
            CASE 6:  // Export Rankings
                sorted = ranking_sort(teams)
                file_save_rankings(sorted, RANKINGS_FILE)
                FREE sorted array
                
            CASE 7:  // Exit
                file_save_teams(teams, TEAMS_FILE)
                file_save_matches(matches, MATCHES_FILE)
                FREE all linked lists
                RETURN 0
```

---

### 2. Team Module (`team.c`)

```
FUNCTION team_create(id, name) -> Team*:
    t = CALLOC(sizeof(Team))
    t->id = id
    t->name = COPY(name)
    t->ranking_points = 0
    t->wins = t->ties = t->losses = 0
    t->total_score = 0
    t->next = NULL
    RETURN t

FUNCTION team_find(head, target_id) -> Team*:
    curr = head
    WHILE curr IS NOT NULL:
        IF curr->id == target_id:
            RETURN curr
        curr = curr->next
    RETURN NULL

FUNCTION team_update_stats(team, my_score, opp_score):
    team->matches_played += 1
    team->total_score += my_score
    
    IF my_score > opp_score:
        team->wins += 1
        team->ranking_points += RP_WIN (3)
    ELSE IF my_score == opp_score:
        team->ties += 1
        team->ranking_points += RP_TIE (1)
    ELSE:
        team->losses += 1
        // RP_LOSS = 0
```

---

### 3. Match Module (`match.c`)

```
FUNCTION match_create(num, red_id, blue_id) -> Match*:
    m = CALLOC(sizeof(Match))
    m->match_num = num
    m->red_team_id = red_id
    m->blue_team_id = blue_id
    m->status = MATCH_PENDING
    RETURN m

FUNCTION match_enter_score(m, teams, red_score, blue_score):
    IF m IS NULL OR m->status == PLAYED:
        RETURN
        
    m->red_score = red_score
    m->blue_score = blue_score
    m->status = MATCH_PLAYED
    
    // Find and update both teams
    red_team = team_find(teams, m->red_team_id)
    blue_team = team_find(teams, m->blue_team_id)
    
    team_update_stats(red_team, red_score, blue_score)
    team_update_stats(blue_team, blue_score, red_score)
    
    // Determine winner
    IF red_score > blue_score:
        winner = "RED"
    ELSE IF blue_score > red_score:
        winner = "BLUE"
    ELSE:
        winner = "TIE"
    
    // Log result
    file_append_result(RESULTS_FILE, m, winner)

FUNCTION match_print_all(head, teams):
    PRINT header row
    curr = head
    WHILE curr IS NOT NULL:
        red = team_find(teams, curr->red_team_id)
        blue = team_find(teams, curr->blue_team_id)
        
        IF curr->status == PLAYED:
            result = GET_RESULT_STRING(curr)
            PRINT match_num, red->name, blue->name, scores, result
        ELSE:
            PRINT match_num, red->name, blue->name, "? - ?", "PENDING"
        
        curr = curr->next
```

---

### 4. Queue Module (`queue.c`)

```
FUNCTION queue_init(q):
    q->front = NULL
    q->rear = NULL
    q->size = 0

FUNCTION queue_enqueue(q, match):
    node = CALLOC(sizeof(QueueNode))
    node->match = match
    node->next = NULL
    
    IF q->rear IS NULL:  // Empty queue
        q->front = node
        q->rear = node
    ELSE:
        q->rear->next = node
        q->rear = node
    
    q->size += 1

FUNCTION queue_dequeue(q) -> Match*:
    IF q->front IS NULL:
        RETURN NULL  // Queue empty
    
    node = q->front
    match = node->match
    q->front = node->next
    
    IF q->front IS NULL:  // Queue now empty
        q->rear = NULL
    
    FREE node
    q->size -= 1
    RETURN match

FUNCTION queue_build_pending(q, matches):
    queue_init(q)
    curr = matches
    WHILE curr IS NOT NULL:
        IF curr->status == MATCH_PENDING:
            queue_enqueue(q, curr)
        curr = curr->next
```

---

### 5. Ranking Module (`ranking.c`)

```
FUNCTION compare_teams(a, b) -> int:
    ta = (Team*)a, tb = (Team*)b
    
    // Primary: Ranking Points (descending)
    IF tb->rp != ta->rp:
        RETURN tb->rp - ta->rp
    
    // Secondary: Total Score (descending)
    IF tb->total_score != ta->total_score:
        RETURN tb->total_score - ta->total_score
    
    // Tertiary: Team ID (ascending)
    RETURN ta->id - tb->id

FUNCTION ranking_sort(head, count) -> Team**:
    // Count teams
    *count = 0
    curr = head
    WHILE curr IS NOT NULL:
        (*count)++
        curr = curr->next
    
    IF *count == 0:
        RETURN NULL
    
    // Create array of team pointers
    arr = MALLOC(*count * sizeof(Team*))
    i = 0
    curr = head
    WHILE curr IS NOT NULL:
        arr[i] = curr
        i++
        curr = curr->next
    
    // Sort using qsort
    QSORT(arr, *count, sizeof(Team*), compare_teams)
    RETURN arr

FUNCTION ranking_display(sorted, count):
    IF sorted IS NULL OR count == 0:
        PRINT "No teams"
        RETURN
    
    PRINT header
    FOR i = 0 TO count-1:
        t = sorted[i]
        PRINT i+1, t->id, t->name, t->rp, t->w, t->t, t->l, t->total_score
```

---

### 6. File I/O Module (`file_io.c`)

```
FUNCTION file_load_teams(path, count) -> Team*:
    file = FOPEN(path, "r")
    IF file IS NULL:
        RETURN NULL
    
    head = NULL, tail = NULL
    SKIP_HEADER_LINE(file)
    
    WHILE READ_LINE(file, line):
        PARSE id, name FROM line
        t = team_create(id, name)
        
        IF head IS NULL:
            head = t, tail = t
        ELSE:
            tail->next = t
            tail = t
        
        (*count)++
    
    FCLOSE(file)
    RETURN head

FUNCTION file_save_teams(head, path):
    file = FOPEN(path, "w")
    WRITE_HEADER(file)
    
    curr = head
    WHILE curr IS NOT NULL:
        WRITE_LINE(curr->id, curr->name, curr->stats...)
        curr = curr->next
    
    FCLOSE(file)

FUNCTION file_append_result(path, match_num, red_id, blue_id, scores, winner):
    file = FOPEN(path, "a")
    IF file IS NULL:
        file = FOPEN(path, "w")  // Create if not exists
        WRITE_HEADER(file)
    
    WRITE_LINE(match_num, red_id, blue_id, scores, winner)
    FCLOSE(file)

FUNCTION file_load_matches(path, count) -> Match*:
    // Similar to file_load_teams
    // Parse: match_num, red_id, blue_id, red_score, blue_score, status
```

---

## Data Flow Summary

```
┌─────────────────────────────────────────────────────────────────────┐
│                        COMPLETE DATA FLOW                           │
└─────────────────────────────────────────────────────────────────────┘

  CSV FILES                    IN-MEMORY STRUCTURES
  ─────────                    ──────────────────────
  
  teams.csv  ────────┐
                     ├──► Team* linked list ──────► ranking_sort()
  matches.csv ───┐   │                              │
                 │   │                              ▼
                 │   │                        Team** sorted array
                 │   │                              │
                 │   │                              ▼
                 │   │                        rankings.csv (export)
                 │   │
                 │   └──────────────────────────► team_update_stats()
                 │                                ▲
                 │                                │
                 └──► Match* linked list ────────►│
                      │                          │
                      │  queue_build_pending()   │
                      ▼                          │
                  MatchQueue (FIFO)              │
                      │                          │
                      │  queue_dequeue()         │
                      ▼                          │
                  PLAY MATCH                     │
                      │                          │
                      │  match_enter_score() ────┘
                      │
                      ▼
                  results.csv (append)
```

---

## License

Educational project for learning C programming and data structures.
