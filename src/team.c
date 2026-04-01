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

void team_free_list(Team *head) {
    Team *curr = head;
    while (curr) {
        Team *next = curr->next;
        free(curr);
        curr = next;
    }
}
