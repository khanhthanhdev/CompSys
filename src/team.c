#include "../include/tournament.h"

Team *team_create(int id, const char *name) {
    Team *t = (Team *)calloc(1, sizeof(Team));
    if (!t) return NULL;
    t->id = id;
    strncpy(t->name, name, MAX_NAME_LEN - 1);
    t->name[MAX_NAME_LEN - 1] = '\0';
    return t;
}

Team *team_find(Team *head, int id) {
    Team *curr = head;
    while (curr) {
        if (curr->id == id) return curr;
        curr = curr->next;
    }
    return NULL;
}

void team_print_all(Team *head) {
    printf("\n%-10s %-20s\n", "Team ID", "Name");
    printf("----------------------------------------\n");
    Team *curr = head;
    while (curr) {
        printf("%-10d %-20s\n", curr->id, curr->name);
        curr = curr->next;
    }
}

void team_update_stats(Team *t, int my_score, int opp_score) {
    if (!t) return;
    t->matches_played++;
    t->total_score += my_score;
    if (my_score > opp_score) {
        t->wins++;
        t->ranking_points += RP_WIN;
    } else if (my_score == opp_score) {
        t->ties++;
        t->ranking_points += RP_TIE;
    } else {
        t->losses++;
        t->ranking_points += RP_LOSS;
    }
}

void team_reset_stats(Team *head) {
    Team *curr = head;
    while (curr) {
        curr->ranking_points = 0;
        curr->matches_played = 0;
        curr->wins = 0;
        curr->ties = 0;
        curr->losses = 0;
        curr->total_score = 0;
        curr = curr->next;
    }
}

void team_rebuild_stats_from_matches(Team *teams, Match *matches) {
    if (!teams) return;

    team_reset_stats(teams);

    Match *curr = matches;
    while (curr) {
        if (curr->status == MATCH_PLAYED) {
            Team *red_team = team_find(teams, curr->red_team_id);
            Team *blue_team = team_find(teams, curr->blue_team_id);

            if (red_team) team_update_stats(red_team, curr->red_score, curr->blue_score);
            if (blue_team) team_update_stats(blue_team, curr->blue_score, curr->red_score);
        }
        curr = curr->next;
    }
}

void team_free_list(Team *head) {
    Team *curr = head;
    while (curr) {
        Team *next = curr->next;
        free(curr);
        curr = next;
    }
}
