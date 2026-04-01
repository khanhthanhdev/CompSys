# Product Requirements Document
## FTC Tournament Manager — Console Application in C

---

## 1. Overview

A console-based backend tool for managing a **FIRST Tech Challenge (FTC)**-style robotics tournament. The program handles team registration, match scheduling, score entry, and live rankings — all persisted through CSV files.

---

## 2. Project File Structure

```
rms_project/
├── Makefile
├── data/
│   ├── teams.csv          # Team registry (input)
│   ├── matches.csv        # Match schedule (input / generated)
│   ├── results.csv        # Scored match results (output)
│   └── rankings.csv       # Current standings (output)
├── include/
│   └── tournament.h       # All structs, enums, and function declarations
└── src/
    ├── main.c             # Entry point, main menu loop
    ├── team.c             # Team CRUD + stats logic
    ├── match.c            # Match loading, display, scoring
    ├── queue.c            # Singly linked list queue (match queue)
    ├── ranking.c          # Ranking calculation + display
    └── file_io.c          # All CSV read/write operations
```

---

## 3. Data Types

### 3.1 Enums

```c
/* Match completion state */
typedef enum {
    MATCH_PENDING = 0,   /* Not yet played */
    MATCH_PLAYED  = 1    /* Score has been entered */
} MatchStatus;

/* Outcome from one team's perspective */
typedef enum {
    OUTCOME_WIN  = 0,
    OUTCOME_TIE  = 1,
    OUTCOME_LOSS = 2
} MatchOutcome;
```

---

### 3.2 Core Structs

#### `Team` — represents one registered team

```c
typedef struct Team {
    int    id;                    /* Unique team number (e.g. 12345) */
    char   name[MAX_NAME_LEN];    /* Team display name */
    int    ranking_points;        /* Cumulative RP: win=3, tie=1, loss=0 */
    int    matches_played;        /* Total matches completed */
    int    wins;
    int    ties;
    int    losses;
    int    total_score;           /* Sum of all scores (tiebreaker) */
    struct Team *next;            /* Next node in singly linked list */
} Team;
```

#### `Match` — represents one qualification match

```c
typedef struct Match {
    int         match_num;        /* Sequential match number (1, 2, 3 …) */
    int         red_team_id;      /* Red alliance team ID */
    int         blue_team_id;     /* Blue alliance team ID */
    int         red_score;        /* Score entered after match */
    int         blue_score;
    MatchStatus status;           /* PENDING or PLAYED */
    struct Match *next;           /* Next node in singly linked list */
} Match;
```

#### `QueueNode` — node wrapping a pending match (for the match queue)

```c
typedef struct QueueNode {
    Match            *match;      /* Pointer to match in master list */
    struct QueueNode *next;
} QueueNode;
```

#### `MatchQueue` — singly linked list queue of upcoming matches

```c
typedef struct {
    QueueNode *front;             /* Dequeue from front */
    QueueNode *rear;              /* Enqueue at rear */
    int        size;
} MatchQueue;
```

---

### 3.3 Constants

```c
#define MAX_NAME_LEN   64
#define MAX_LINE_LEN  256

#define RP_WIN    3
#define RP_TIE    1
#define RP_LOSS   0

#define TEAMS_FILE    "data/teams.csv"
#define MATCHES_FILE  "data/matches.csv"
#define RESULTS_FILE  "data/results.csv"
#define RANKINGS_FILE "data/rankings.csv"
```

---

## 4. Module & Function Definitions

### 4.1 `team.c` — Team management

| Function | Signature | Description |
|---|---|---|
| Create | `Team *team_create(int id, const char *name)` | `malloc` a new Team node, zero-init stats |
| Find by ID | `Team *team_find(Team *head, int id)` | Walk list, return pointer or NULL |
| Print all | `void team_print_all(Team *head)` | Formatted table to stdout |
| Update stats | `void team_update_stats(Team *t, int my_score, int opp_score)` | Increment wins/ties/losses, RP, total_score |
| Free list | `void team_free_list(Team *head)` | `free` every node |

---

### 4.2 `match.c` — Match logic

| Function | Signature | Description |
|---|---|---|
| Create | `Match *match_create(int num, int red_id, int blue_id)` | `malloc` a Match, status = PENDING |
| Find by number | `Match *match_find(Match *head, int num)` | Linear search through list |
| Enter score | `void match_enter_score(Match *m, Team *teams, int red_score, int blue_score)` | Set scores, status = PLAYED, update both teams' stats |
| Print all | `void match_print_all(Match *head, Team *teams)` | Table with team names, scores, status |
| Print one | `void match_print_one(Match *m, Team *teams)` | Single match detail row |
| Free list | `void match_free_list(Match *head)` | `free` every node |

---

### 4.3 `queue.c` — Match Queue (Singly Linked List)

> Implements the **Queue** data structure required by the assignment (Week 5).

| Function | Signature | Description |
|---|---|---|
| Init | `void queue_init(MatchQueue *q)` | Set front/rear = NULL, size = 0 |
| Enqueue | `void queue_enqueue(MatchQueue *q, Match *m)` | `malloc` QueueNode, append to rear |
| Dequeue | `Match *queue_dequeue(MatchQueue *q)` | Remove from front, return Match pointer, `free` node |
| Peek | `Match *queue_peek(MatchQueue *q)` | Return front match without removing |
| Is empty | `int queue_is_empty(MatchQueue *q)` | Return 1 if size == 0 |
| Build pending | `void queue_build_pending(MatchQueue *q, Match *head)` | Enqueue all PENDING matches in order |
| Free | `void queue_free(MatchQueue *q)` | Free all remaining QueueNode allocations |

---

### 4.4 `ranking.c` — Rankings

| Function | Signature | Description |
|---|---|---|
| Sort | `Team **ranking_sort(Team *head, int count)` | Build `malloc`'d pointer array, sort by RP desc → total_score desc → id asc |
| Display | `void ranking_display(Team **sorted, int count)` | Print formatted standings table |
| Free sorted | `void ranking_free_sorted(Team **arr)` | `free` the pointer array (not the Team nodes) |

**Tiebreaker order:**
1. Ranking Points (RP) — higher wins
2. Total cumulative score — higher wins
3. Team ID — lower wins (stable)

---

### 4.5 `file_io.c` — CSV Persistence

| Function | Signature | Description |
|---|---|---|
| Load teams | `Team *file_load_teams(const char *path, int *count)` | Parse `id,name` CSV → linked list |
| Save teams | `void file_save_teams(Team *head, const char *path)` | Write full team stats CSV |
| Load matches | `Match *file_load_matches(const char *path, int *count)` | Parse `match_num,red_id,blue_id,red_score,blue_score,status` |
| Save matches | `void file_save_matches(Match *head, const char *path)` | Write all match records (overwrites file) |
| Save rankings | `void file_save_rankings(Team **sorted, int count, const char *path)` | Write current sorted standings |

---

### 4.6 `main.c` — Entry Point & Menu

**Program flow:**

```
startup
  ├─ file_load_teams()       → Team linked list
  ├─ file_load_matches()     → Match linked list
  └─ queue_build_pending()   → MatchQueue of unplayed matches

main menu loop
  [1] View Teams
  [2] View Match Schedule
  [3] Play Next Match        ← queue_dequeue → match_enter_score → file_save_matches
  [4] Enter Score by Match # ← match_find → match_enter_score → file_save_matches
  [5] View Results           ← match_print_all (PLAYED only)
  [6] View Rankings          ← ranking_sort → ranking_display → file_save_rankings
  [7] Save & Exit            ← file_save_teams → file_save_matches → free all memory
```

---

## 5. CSV File Formats

### `teams.csv` (input)
```
id,name
12345,Iron Wolves
67890,Quantum Gears
11111,Robo Rockets
```

### `matches.csv` (input + updated on score entry)
```
match_num,red_team_id,blue_team_id,red_score,blue_score,status
1,12345,67890,0,0,0
2,11111,12345,0,0,0
3,67890,11111,0,0,0
```
> `status`: `0` = PENDING, `1` = PLAYED

### `rankings.csv` (auto-generated output)
```
rank,team_id,name,rp,wins,ties,losses,total_score
1,12345,Iron Wolves,6,2,0,0,320
2,67890,Quantum Gears,3,1,0,1,280
```

---

## 6. Ranking Points Rule

| Match Result | RP Awarded |
|---|---|
| Win (`my_score > opp_score`) | **3** |
| Tie (`my_score == opp_score`) | **1** |
| Loss (`my_score < opp_score`) | **0** |

Both alliances (red and blue) are evaluated independently after each match.

---

## 7. Technical Checklist (Assignment Requirements)

| Requirement | Implementation |
|---|---|
| ≥ 300 lines of code | Across 6 `.c` files |
| ≥ 5 core functions | `team_update_stats`, `match_enter_score`, `queue_enqueue`, `queue_dequeue`, `ranking_sort`, `file_load_*` |
| Struct + Pointers | `Team`, `Match`, `QueueNode` — all manipulated via pointers |
| Dynamic memory (`malloc`) | Every node in Team list, Match list, and MatchQueue |
| Singly Linked List Queue | `MatchQueue` in `queue.c` with enqueue/dequeue |
| File I/O (read + write) | All CSV load/save in `file_io.c` |
| Multiple modules (`.c`/`.h`) | 6 source files + 1 shared header |
| Makefile | Compiles all `.c` files, supports `clean` target |

---

## 8. Makefile

```makefile
CC      = gcc
CFLAGS  = -Wall -Wextra -Iinclude
SRC     = src/main.c src/team.c src/match.c src/queue.c src/ranking.c src/file_io.c
OBJ     = $(SRC:.c=.o)
TARGET  = tournament

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
```