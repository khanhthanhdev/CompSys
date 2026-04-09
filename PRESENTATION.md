# FTC Tournament Management System — Presentation Script

**Total Duration:** ~18–20 minutes

---

## Phase 1: Introduction & Architecture (4–5 minutes)

---

### Slide 1 — Title Slide (1 min)

**Speaker says:**

> "Good [morning/afternoon], we are [Name A — Student ID: XXXXX] and [Name B — Student ID: XXXXX].
>
> Today we present our project: **A Console-Based Robotics Tournament Management System** — or **RMS** for short.
>
> The problem it solves: Running an FTC robotics tournament involves tracking teams, scheduling matches, recording live scores, and computing rankings — all in real time. Our tool automates this entire workflow from a single terminal, with persistent CSV storage, so a tournament organiser needs nothing more than a laptop."

---

### Slide 2 — System Overview & Flowchart (2 mins) [Report §3.1]

**Display:** The Overall Flowchart from `TECHNICAL_REPORT.md` §3.1 (Fig 1).

```
Program Start → Init variables → Welcome → Main Menu
                                              │
     ┌────────┬────────┬────────┬────────┬────┴───┬────────┐
     1        2        3        4        5        6        7
  Import   Import   Play    View     View    Export    Exit
  Teams   Schedule  Match  Schedule Rankings Rankings  (Save+Free)
     └────────┴────────┴────────┴────────┴─────────┴────────┘
                            ↑       loop back to menu       ↑
```

**Speaker says:**

> "Here is the overall system flow. When the program starts (`main.c` line 18), it initialises all variables to `NULL`, creates an empty queue, and enters the main menu loop.
>
> A typical user journey looks like this:
> 1. **Option 1** — Import teams from `data/teams.csv` (loads 10 teams into a linked list).
> 2. **Option 2** — Import the match schedule from `data/matches.csv` (loads 20 matches; pending ones go into a FIFO queue).
> 3. **Option 3** — Play the next match: the queue dequeues a match, prompts for scores, updates both teams' stats, and appends the result to `data/results.csv`.
> 4. **Option 5** — View live rankings sorted by Ranking Points.
> 5. **Option 7** — Exit: the program saves updated team and match data back to CSV, then frees all heap memory before terminating.
>
> The loop only breaks on Option 7, so the tournament can run for as many matches as needed."

---

### Slide 3 — Module Architecture (2 mins) [Report §3.2, Req 6]

**Display:** The Module Architecture Diagram from `TECHNICAL_REPORT.md` §3.2 (Fig 2).

```
                ┌──────────────────────────┐
                │  tournament.h (Header)   │
                │  Structs, Enums, Protos  │
                └──────────┬───────────────┘
          ┌────────┬───────┼───────┬──────────┬──────────┐
          ▼        ▼       ▼       ▼          ▼          ▼
       main.c   team.c  match.c  queue.c  ranking.c  file_io.c
       (UI)     (CRUD)  (Score)  (FIFO)   (Sort)     (CSV I/O)
                                                        │
                                              ┌─────────┼─────────┐
                                              ▼         ▼         ▼
                                         teams.csv  matches.csv  results.csv
```

**Speaker says:**

> "We split the code into **6 source files and 1 shared header** — 603 lines total.
>
> - `tournament.h` — the single header that declares all structs (`Team`, `Match`, `QueueNode`, `MatchQueue`), enums, and every function prototype. Every `.c` file includes this one header.
> - `main.c` (97 lines) — handles **only** the UI: printing the menu and dispatching user choices. It contains zero business logic.
> - `team.c` (54 lines) — Team linked-list operations: create, find, update stats, free.
> - `match.c` (86 lines) — Match linked-list operations: create, find, enter scores, display, free.
> - `queue.c` (66 lines) — A standalone FIFO queue module: init, enqueue, dequeue, peek, build-pending, free.
> - `ranking.c` (58 lines) — Sorting teams with `qsort()` and a custom comparator, plus display.
> - `file_io.c` (138 lines) — All CSV read/write/append operations isolated here.
>
> The key design principle is **separation of concerns**: `main.c` never touches a file, and `file_io.c` never prints to the console.
>
> We automate compilation with a **Makefile** (line 1–18). Running `make` compiles each `.c` into an `.o`, then links them into the `tournament` binary. The flags `-Wall -Wextra` enable strict warnings so we catch issues at compile time."

---

## Phase 2: Technical Deep Dive (7–8 minutes)

---

### Slide 4 — Structs & Memory Management (2 mins) [Req 2 & 3]

**Display code snippet — `tournament.h` lines 34–65:**

```c
// tournament.h lines 34-44
typedef struct Team {
    int    id;
    char   name[MAX_NAME_LEN];   // 64 bytes
    int    ranking_points;
    int    matches_played;
    int    wins, ties, losses;
    int    total_score;
    struct Team *next;           // ← singly linked list pointer
} Team;

// tournament.h lines 46-54
typedef struct Match {
    int         match_num;
    int         red_team_id;
    int         blue_team_id;
    int         red_score;
    int         blue_score;
    MatchStatus status;          // PENDING (0) or PLAYED (1)
    struct Match *next;          // ← singly linked list pointer
} Match;

// tournament.h lines 56-65
typedef struct QueueNode {
    Match            *match;     // ← pointer to existing Match node
    struct QueueNode *next;
} QueueNode;

typedef struct {
    QueueNode *front;
    QueueNode *rear;
    int        size;
} MatchQueue;
```

```c
Match *m = (Match *)calloc(1, sizeof(Match));
QueueNode *node = (QueueNode *)malloc(sizeof(QueueNode));
Team **arr = (Team **)malloc(*count * sizeof(Team *));
Team *t = (Team *)calloc(1, sizeof(Team));

ranking_free_sorted(sorted);
team_free_list(teams);
match_free_list(matches);
queue_free(&queue);
```

**Speaker says:**

> "These are our three core structs. Notice every struct has a `next` pointer — this is how we build **singly linked lists** without any static arrays for the main data.
>
> Now let's look at **dynamic memory allocation**:
>
> - `team.c` **line 4**: `Team *t = (Team *)calloc(1, sizeof(Team));` — we use `calloc` so all stats start at zero.
> - `match.c` **line 4**: `Match *m = (Match *)calloc(1, sizeof(Match));` — same pattern.
> - `queue.c` **line 10**: `QueueNode *node = (QueueNode *)malloc(sizeof(QueueNode));` — `malloc` for queue nodes.
> - `ranking.c` **line 24**: `Team **arr = (Team **)malloc(*count * sizeof(Team *));` — dynamically-sized pointer array for sorting.
>
> **No static arrays** are used for the main database — everything lives on the heap.
>
> For **deallocation**, every module has a `free` function:
> - `team_free_list()` at `team.c` lines 47–54: walks the linked list, frees each node.
> - `match_free_list()` at `match.c` lines 79–86: identical pattern.
> - `queue_free()` at `queue.c` lines 58–66: frees only the `QueueNode` wrappers (not the underlying `Match` nodes, because they belong to the match list).
> - `ranking_free_sorted()` at `ranking.c` lines 56–58: frees the temporary pointer array.
>
> On exit (`main.c` lines 86–92), we call all four free functions before returning — **zero memory leaks**."

---

### Slide 5 — Data Structure Implementation (3 mins) [Req 4]

**Display side-by-side: Pseudocode (Report §3.3) and C code.**

**Pseudocode — Queue Enqueue (Report §3.3, Algorithm 3):**
```
FUNCTION queue_enqueue(q, match):
    node ← MALLOC(sizeof(QueueNode))
    node.match = match
    node.next  = NULL

    IF q.rear IS NULL:        // empty queue
        q.front = node
        q.rear  = node
    ELSE:
        q.rear.next = node
        q.rear      = node

    q.size += 1
```

**C Code — `queue.c` lines 9–22:**
```c
void queue_enqueue(MatchQueue *q, Match *m) {
    QueueNode *node = (QueueNode *)malloc(sizeof(QueueNode));
    if (!node) return;
    node->match = m;
    node->next = NULL;

    if (q->rear) {
        q->rear->next = node;
    } else {
        q->front = node;
    }
    q->rear = node;
    q->size++;
}
```

**Speaker says:**

> "We implemented a **Singly Linked List** used as a **FIFO Queue** to manage the match schedule.
>
> Why a Queue? Because in a real FTC tournament, matches are played **first-in, first-out** — the first scheduled match is the first one played. Our `MatchQueue` enforces this ordering.
>
> Let me walk through the enqueue operation:
> 1. We `malloc` a new `QueueNode` (line 10).
> 2. We store a pointer to the existing `Match` node — we do **not** copy the match data.
> 3. If the queue is empty (`q->rear` is `NULL`), both `front` and `rear` point to the new node.
> 4. Otherwise, we link the current `rear->next` to the new node, then advance `rear`.
> 5. Increment `size`.
>
> **Dequeue** (`queue.c` lines 24–37) mirrors this: we remove from `front`, advance `front` to `front->next`, free the wrapper node, and return the `Match*`. If the queue becomes empty, we reset `rear` to `NULL`.
>
> **Building the queue** (`queue_build_pending`, lines 47–56): after loading all matches from CSV, we traverse the match linked list and enqueue only those with `status == MATCH_PENDING`. This is O(n) and runs once at import.
>
> **Time complexities:**
> - Enqueue: O(1)
> - Dequeue: O(1)
> - Build pending: O(n)
> - Peek: O(1)"

---

### Slide 6 — Advanced Pointers & File I/O (2–3 mins) [Req 1, 2, & 5]

**Display code — Pointer usage:**

```c
// ranking.c — Double pointer dereferencing
static int compare_teams(const void *a, const void *b) {
    Team *ta = *(Team **)a;   // Dereference
    Team *tb = *(Team **)b;
    // ...
}

// ranking.c  — Pointer array + qsort
Team **arr = (Team **)malloc(*count * sizeof(Team *));
int i = 0;
curr = head;
while (curr) {
    arr[i++] = curr;   // arr[i] is a pointer to a Team node
    curr = curr->next; // Traverse via pointer
}
qsort(arr, *count, sizeof(Team *), compare_teams);

// team.c — Pointer-based linked list traversal
Team *team_find(Team *head, int id) {
    Team *curr = head;
    while (curr) {
        if (curr->id == id) return curr;
        curr = curr->next;  // Advance pointer to next node
    }
    return NULL;
}

// Traversal
curr = curr->next;
// Pointer Arrays for Sorting
Team **arr = malloc(count * sizeof(Team*));
qsort(arr, count, sizeof(Team*), cmp);
// Double Pointer Dereferencing
Team *t = *(Team **)a;
```

**Speaker says:**

> "Pointers are central to this project. Three key patterns:
>
> 1. **Linked list traversal** (`team.c` line 14): `curr = curr->next` — we walk the list using the `next` pointer until we hit `NULL`.
> 2. **Pointer arrays** (`ranking.c` line 24): We `malloc` an array of `Team*` pointers, fill it by traversing the linked list, then pass it to `qsort`. This lets us sort without modifying the original list.
> 3. **Double-pointer dereferencing** (`ranking.c` line 4): `qsort` passes `const void*` to the comparator. We cast to `Team**` and dereference to get the actual `Team*`.
>
> Now, **File I/O** — all in `file_io.c`:
>
> - **Reading** (`file_io.c` lines 3–40): We `fopen` in `"r"` mode, skip the CSV header with an initial `fgets`, then loop `fgets` line by line. Each line is parsed with `sscanf(line, "%d,%63[^,\\n]", &id, name)` — the format string `%63[^,\\n]` reads up to 63 characters until a comma or newline, preventing buffer overflow. For each parsed row, we call `team_create()` which `calloc`s a new Team node, and we append it to the tail of the linked list.
>
> - **Writing** (`file_io.c` lines 42–55): We `fopen` in `"w"` mode (overwrite), write a CSV header with `fprintf`, then traverse the linked list writing each node's fields. This **serialises the in-memory linked list back to disk**.
>
> - **Appending** (`file_io.c` lines 128–138): `file_append_result` opens in `"a"` mode. If the file doesn't exist yet (`fopen("a")` returns `NULL`), it falls back to `"w"` mode and writes a header first. Each match result is appended as a single CSV row — this gives us an **audit log** of every match played.
>
> The key insight: on exit (`main.c` lines 86–87), we call `file_save_teams()` and `file_save_matches()` to persist the entire state. On next boot, `file_load_teams()` and `file_load_matches()` reconstruct the linked lists from those CSV files — **the dynamic memory state survives across sessions.**"

---

## Phase 3: Live Demonstration (4–5 minutes)

---

### Demo Script (Screen-share terminal)

**Speaker says:**

> "Let me now demonstrate the system live."

**Step 1: Build (show Makefile works)**
```bash
$ make clean
rm -f src/*.o tournament

$ make
gcc -Wall -Wextra -Iinclude -c src/main.c -o src/main.o
gcc -Wall -Wextra -Iinclude -c src/team.c -o src/team.o
gcc -Wall -Wextra -Iinclude -c src/match.c -o src/match.o
gcc -Wall -Wextra -Iinclude -c src/queue.c -o src/queue.o
gcc -Wall -Wextra -Iinclude -c src/ranking.c -o src/ranking.o
gcc -Wall -Wextra -Iinclude -c src/file_io.c -o src/file_io.o
gcc -Wall -Wextra -Iinclude -o tournament src/main.o src/team.o src/match.o src/queue.o src/ranking.o src/file_io.o
```

> "Notice: zero warnings with `-Wall -Wextra`. The Makefile compiles each `.c` into a `.o` object file, then links them into the `tournament` binary."

**Step 2: Run and demonstrate 5+ core functions**

```bash
$ ./tournament
Welcome to FTC Tournament Manager!
```

| # | Action | Menu Option | What to say |
|---|--------|-------------|-------------|
| 1 | **Import Teams** | Option 1 | "We load 10 teams from `data/teams.csv` into a linked list. Each team is `calloc`'d on the heap." |
| 2 | **Import Match Schedule** | Option 2 | "20 matches loaded. The system finds pending matches and enqueues them into our FIFO queue. Here it shows N pending." |
| 3 | **Play a Match (score entry)** | Option 3 | "The queue dequeues the next pending match. I enter RED score = 85, BLUE score = 72. Both teams' stats are updated in memory, and the result is appended to `results.csv`." |
| 4 | **View Schedule & Results** | Option 4 | "All 20 matches displayed. Played matches show scores and winner. The match we just played now shows the new scores." |
| 5 | **View Rankings** | Option 5 | "Teams are sorted by Ranking Points (RP), then total score as tiebreaker. The team that just won has moved up." |
| 6 | **Export Rankings** | Option 6 | "Rankings exported to `data/rankings.csv`." |
| 7 | **Exit** | Option 7 | "On exit, teams and matches are saved back to their CSV files, all memory is freed, and we get 'Goodbye!'." |

**Step 3: File I/O Proof**

> "Let me now open the actual data files to prove persistence."

```bash
$ cat data/results.csv
match_num,red_team_id,blue_team_id,red_score,blue_score,winner
19,1003,1010,85,72,RED

$ cat data/rankings.csv
rank,team_id,team_name,rp,wins,ties,losses,total_score
1,1001,Red Dragons,9,3,0,0,312
...

$ cat data/teams.csv
team_id,team_name,rp,wins,ties,losses,total_score
1001,Red Dragons,9,3,0,0,312
...
```

> "As you can see, all data was permanently saved. If I restart the program and import again, it reconstructs the exact same state from these files."

---

## Phase 4: Testing & Conclusion (2–3 minutes)

---

### Slide 7 — Testing & Edge Cases (1.5 mins) [Report §4.0]

**Display: Testing Table from `TECHNICAL_REPORT.md` §4.1**

| Test ID | Feature | Input | Expected Output | Status |
|---------|---------|-------|-----------------|--------|
| TC-01 | Import Teams | Option 1, valid CSV | "Loaded N teams" | ✅ PASS |
| TC-02 | Import Teams | Missing file | "Warning: Cannot open..." | ✅ PASS |
| TC-04 | Play Match | Teams not loaded | "No teams loaded..." | ✅ PASS |
| TC-05 | Play Match | Empty queue | "All matches have been played" | ✅ PASS |
| TC-11 | Invalid Input | Non-numeric (e.g., "abc") | "Invalid input. Please enter a number." | ✅ PASS |
| TC-12 | Invalid Option | Menu choice 99 | "Invalid option." | ✅ PASS |

**Speaker says:**

> "We tested 12 scenarios covering happy paths and edge cases. Let me highlight two:
>
> **Edge Case 1 — Dequeue from empty queue** (TC-05):
> At `queue.c` line 25: `if (!q->front) return NULL;` — if the queue is empty, `queue_dequeue` safely returns `NULL` instead of crashing with a segfault. Then in `main.c` line 54, we check `if (!m)` and print 'All matches have been played.'
>
> **Edge Case 2 — Non-numeric menu input** (TC-11):
> At `main.c` lines 32–36: `if (scanf("%d", &choice) != 1)` catches non-integer input. We flush the input buffer with `while (getchar() != '\n')` to prevent an infinite loop, then prompt again.
>
> **Style Guide:** Our code follows the Google C++ Style Guide conventions — 2-space logical indentation, `snake_case` function names, descriptive variable names, consistent brace placement, and `typedef` for all struct types."

---

### Slide 8 — Task Delegation & Wrap-up (1 min)

**Speaker says:**

> "Here is how we divided the work:
>
> | Member | Responsibility |
> |--------|----------------|
> | **[Speaker A]** | Linked List implementation (`team.c`, `match.c`), Queue module (`queue.c`), Makefile, project structure |
> | **[Speaker B]** | File I/O module (`file_io.c`), Ranking module (`ranking.c`), Main UI (`main.c`), Testing & Documentation |
>
> Both members reviewed each other's code and contributed to the Technical Report.
>
> **In summary**, our system demonstrates:
> - ✅ Dynamic memory with `malloc`/`calloc` — no static arrays
> - ✅ Singly linked lists for Teams and Matches
> - ✅ FIFO queue for match scheduling
> - ✅ File I/O: read, write, and append to CSV
> - ✅ Modular design across 6 source files + Makefile
> - ✅ 603 lines of code, 25+ functions
> - ✅ Proper memory deallocation — zero leaks
>
> Thank you for watching. We're happy to take any questions."

---

## Quick Reference — File Locations for Slides

| Topic | File | Lines |
|-------|------|-------|
| Team struct | `tournament.h` | 34–44 |
| Match struct | `tournament.h` | 46–54 |
| Queue structs | `tournament.h` | 56–65 |
| `calloc` for Team | `team.c` | 4 |
| `calloc` for Match | `match.c` | 4 |
| `malloc` for QueueNode | `queue.c` | 10 |
| `malloc` for sort array | `ranking.c` | 24 |
| Queue enqueue | `queue.c` | 9–22 |
| Queue dequeue | `queue.c` | 24–37 |
| Queue build pending | `queue.c` | 47–56 |
| Team free list | `team.c` | 47–54 |
| Match free list | `match.c` | 79–86 |
| Queue free | `queue.c` | 58–66 |
| Pointer comparator | `ranking.c` | 3–12 |
| File load teams | `file_io.c` | 3–40 |
| File save teams | `file_io.c` | 42–55 |
| File append result | `file_io.c` | 128–138 |
| Empty queue guard | `queue.c` | 25 |
| Invalid input guard | `main.c` | 32–36 |
| Exit cleanup | `main.c` | 86–92 |
| Makefile | `Makefile` | 1–18 |

curl -fsSL \
    -O https://raw.githubusercontent.com/searxng/searxng/master/container/docker-compose.yml \
    -O https://raw.githubusercontent.com/searxng/searxng/master/container/.env.example