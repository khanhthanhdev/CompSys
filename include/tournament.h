#ifndef TOURNAMENT_H
#define TOURNAMENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Constants */
#define MAX_NAME_LEN   64
#define MAX_LINE_LEN  256

#define RP_WIN    3
#define RP_TIE    1
#define RP_LOSS   0

#define TEAMS_FILE    "data/teams.csv"
#define MATCHES_FILE  "data/matches.csv"
#define RESULTS_FILE  "data/results.csv"
#define RANKINGS_FILE "data/rankings.csv"

/* Enums */
typedef enum {
    MATCH_PENDING = 0,
    MATCH_PLAYED  = 1
} MatchStatus;

typedef enum {
    OUTCOME_WIN  = 0,
    OUTCOME_TIE  = 1,
    OUTCOME_LOSS = 2
} MatchOutcome;

/* Structs */
typedef struct Team {
    int    id;
    char   name[MAX_NAME_LEN];
    int    ranking_points;
    int    matches_played;
    int    wins;
    int    ties;
    int    losses;
    int    total_score;
    struct Team *next;
} Team;

typedef struct Match {
    int         match_num;
    int         red_team_id;
    int         blue_team_id;
    int         red_score;
    int         blue_score;
    MatchStatus status;
    struct Match *next;
} Match;

typedef struct QueueNode {
    Match            *match;
    struct QueueNode *next;
} QueueNode;

typedef struct {
    QueueNode *front;
    QueueNode *rear;
    int        size;
} MatchQueue;

/* Team functions (team.c) */
Team *team_create(int id, const char *name);
Team *team_find(Team *head, int id);
void team_print_all(Team *head);
void team_update_stats(Team *t, int my_score, int opp_score);
void team_reset_stats(Team *head);
void team_rebuild_stats_from_matches(Team *teams, Match *matches);
void team_free_list(Team *head);

/* Match functions (match.c) */
Match *match_create(int num, int red_id, int blue_id);
Match *match_find(Match *head, int num);
void match_enter_score(Match *m, Team *teams, int red_score, int blue_score);
void match_print_all(Match *head, Team *teams);
void match_print_one(Match *m, Team *teams);
void match_free_list(Match *head);

/* Queue functions (queue.c) */
void queue_init(MatchQueue *q);
void queue_enqueue(MatchQueue *q, Match *m);
Match *queue_dequeue(MatchQueue *q);
Match *queue_peek(MatchQueue *q);
int queue_is_empty(MatchQueue *q);
void queue_build_pending(MatchQueue *q, Match *head);
void queue_free(MatchQueue *q);

/* Ranking functions (ranking.c) */
Team **ranking_sort(Team *head, int *count);
void ranking_display(Team **sorted, int count);
void ranking_free_sorted(Team **arr);

/* File I/O functions (file_io.c) */
Team *file_load_teams(const char *path, int *count);
void file_save_teams(Team *head, const char *path);
Match *file_load_matches(const char *path, int *count);
void file_save_matches(Match *head, const char *path);
void file_save_rankings(Team **sorted, int count, const char *path);
void file_append_result(const char *path, int match_num, int red_id, int blue_id, int red_score, int blue_score, const char *winner);

#endif /* TOURNAMENT_H */
