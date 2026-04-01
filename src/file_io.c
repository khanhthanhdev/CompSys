#include "../include/tournament.h"

Team *file_load_teams(const char *path, int *count) {
    FILE *f = fopen(path, "r");
    if (!f) {
        printf("Warning: Cannot open %s\n", path);
        *count = 0;
        return NULL;
    }

    Team *head = NULL;
    Team *tail = NULL;
    char line[MAX_LINE_LEN];
    *count = 0;

    if (fgets(line, sizeof(line), f)) {
        while (fgets(line, sizeof(line), f)) {
            int id;
            char name[MAX_NAME_LEN];
            if (sscanf(line, "%d,%63[^,\n]", &id, name) == 2) {
                Team *t = team_create(id, name);
                if (t) {
                    if (!head) {
                        head = t;
                        tail = t;
                    } else {
                        tail->next = t;
                        tail = t;
                    }
                    (*count)++;
                }
            } else {
                printf("Warning: Skipping malformed row: %s", line);
            }
        }
    }

    fclose(f);
    return head;
}

void file_save_teams(Team *head, const char *path) {
    FILE *f = fopen(path, "w");
    if (!f) return;

    fprintf(f, "team_id,team_name,rp,wins,ties,losses,total_score\n");
    Team *curr = head;
    while (curr) {
        fprintf(f, "%d,%s,%d,%d,%d,%d,%d\n",
                curr->id, curr->name, curr->ranking_points,
                curr->wins, curr->ties, curr->losses, curr->total_score);
        curr = curr->next;
    }
    fclose(f);
}

Match *file_load_matches(const char *path, int *count) {
    FILE *f = fopen(path, "r");
    if (!f) {
        printf("Warning: Cannot open %s\n", path);
        *count = 0;
        return NULL;
    }

    Match *head = NULL;
    Match *tail = NULL;
    char line[MAX_LINE_LEN];
    *count = 0;

    if (fgets(line, sizeof(line), f)) {
        while (fgets(line, sizeof(line), f)) {
            int match_num, red_id, blue_id, red_score, blue_score, status;
            if (sscanf(line, "%d,%d,%d,%d,%d,%d",
                       &match_num, &red_id, &blue_id, &red_score, &blue_score, &status) >= 3) {
                Match *m = match_create(match_num, red_id, blue_id);
                if (m) {
                    m->red_score = red_score;
                    m->blue_score = blue_score;
                    m->status = (MatchStatus)status;
                    if (!head) {
                        head = m;
                        tail = m;
                    } else {
                        tail->next = m;
                        tail = m;
                    }
                    (*count)++;
                }
            } else {
                printf("Warning: Skipping malformed row: %s", line);
            }
        }
    }

    fclose(f);
    return head;
}

void file_save_matches(Match *head, const char *path) {
    FILE *f = fopen(path, "w");
    if (!f) return;

    fprintf(f, "match_num,red_team_id,blue_team_id,red_score,blue_score,status\n");
    Match *curr = head;
    while (curr) {
        fprintf(f, "%d,%d,%d,%d,%d,%d\n",
                curr->match_num, curr->red_team_id, curr->blue_team_id,
                curr->red_score, curr->blue_score, curr->status);
        curr = curr->next;
    }
    fclose(f);
}

void file_save_rankings(Team **sorted, int count, const char *path) {
    FILE *f = fopen(path, "w");
    if (!f) return;

    fprintf(f, "rank,team_id,team_name,rp,wins,ties,losses,total_score\n");
    for (int i = 0; i < count; i++) {
        Team *t = sorted[i];
        fprintf(f, "%d,%d,%s,%d,%d,%d,%d,%d\n",
                i + 1, t->id, t->name, t->ranking_points,
                t->wins, t->ties, t->losses, t->total_score);
    }
    fclose(f);
}

void file_append_result(const char *path, int match_num, int red_id, int blue_id,
                        int red_score, int blue_score, const char *winner) {
    FILE *f = fopen(path, "a");
    if (!f) {
        f = fopen(path, "w");
        if (!f) return;
        fprintf(f, "match_num,red_team_id,blue_team_id,red_score,blue_score,winner\n");
    }
    fprintf(f, "%d,%d,%d,%d,%d,%s\n", match_num, red_id, blue_id, red_score, blue_score, winner);
    fclose(f);
}
