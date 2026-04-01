#include "../include/tournament.h"

Match *match_create(int num, int red_id, int blue_id) {
    Match *m = (Match *)calloc(1, sizeof(Match));
    if (!m) return NULL;
    m->match_num = num;
    m->red_team_id = red_id;
    m->blue_team_id = blue_id;
    m->status = MATCH_PENDING;
    return m;
}

Match *match_find(Match *head, int num) {
    Match *curr = head;
    while (curr) {
        if (curr->match_num == num) return curr;
        curr = curr->next;
    }
    return NULL;
}

void match_enter_score(Match *m, Team *teams, int red_score, int blue_score) {
    if (!m || m->status == MATCH_PLAYED) return;
    m->red_score = red_score;
    m->blue_score = blue_score;
    m->status = MATCH_PLAYED;

    Team *red_team = team_find(teams, m->red_team_id);
    Team *blue_team = team_find(teams, m->blue_team_id);

    if (red_team) team_update_stats(red_team, red_score, blue_score);
    if (blue_team) team_update_stats(blue_team, blue_score, red_score);

    const char *winner;
    if (red_score > blue_score) winner = "RED";
    else if (blue_score > red_score) winner = "BLUE";
    else winner = "TIE";

    file_append_result(RESULTS_FILE, m->match_num, m->red_team_id, m->blue_team_id, red_score, blue_score, winner);
}

void match_print_all(Match *head, Team *teams) {
    printf("\n%-8s %-20s vs %-20s | Score     | Result\n", "Match", "Red", "Blue");
    printf("============================================================================\n");
    Match *curr = head;
    int pending = 0, played = 0;
    while (curr) {
        Team *red = team_find(teams, curr->red_team_id);
        Team *blue = team_find(teams, curr->blue_team_id);
        if (curr->status == MATCH_PLAYED) {
            played++;
            const char *result;
            if (curr->red_score > curr->blue_score) result = "RED WINS";
            else if (curr->blue_score > curr->red_score) result = "BLUE WINS";
            else result = "TIE";
            printf("%-8d %-20s   %-20s | %3d - %-3d | %s\n",
                   curr->match_num,
                   red ? red->name : "Unknown", blue ? blue->name : "Unknown",
                   curr->red_score, curr->blue_score, result);
        } else {
            pending++;
            printf("%-8d %-20s   %-20s |   ? - ?   | PENDING\n",
                   curr->match_num,
                   red ? red->name : "Unknown", blue ? blue->name : "Unknown");
        }
        curr = curr->next;
    }
    printf("------------------------------------------------------------------------\n");
    printf("Total: %d matches (%d played, %d pending)\n", played + pending, played, pending);
}

void match_print_one(Match *m, Team *teams) {
    Team *red = team_find(teams, m->red_team_id);
    Team *blue = team_find(teams, m->blue_team_id);
    printf("%s (RED) vs %s (BLUE)\n",
           red ? red->name : "Unknown", blue ? blue->name : "Unknown");
}

void match_free_list(Match *head) {
    Match *curr = head;
    while (curr) {
        Match *next = curr->next;
        free(curr);
        curr = next;
    }
}
