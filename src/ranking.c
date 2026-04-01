#include "../include/tournament.h"

static int compare_teams(const void *a, const void *b) {
    Team *ta = *(Team **)a;
    Team *tb = *(Team **)b;

    if (tb->ranking_points != ta->ranking_points)
        return tb->ranking_points - ta->ranking_points;
    if (tb->total_score != ta->total_score)
        return tb->total_score - ta->total_score;
    return ta->id - tb->id;
}

Team **ranking_sort(Team *head, int *count) {
    *count = 0;
    Team *curr = head;
    while (curr) {
        (*count)++;
        curr = curr->next;
    }

    if (*count == 0) return NULL;

    Team **arr = (Team **)malloc(*count * sizeof(Team *));
    if (!arr) return NULL;

    int i = 0;
    curr = head;
    while (curr) {
        arr[i++] = curr;
        curr = curr->next;
    }

    qsort(arr, *count, sizeof(Team *), compare_teams);
    return arr;
}

void ranking_display(Team **sorted, int count) {
    if (!sorted || count == 0) {
        printf("No teams to display.\n");
        return;
    }

    printf("\n%-6s %-8s %-20s %-5s %-5s %-5s %-5s %-10s\n",
           "Rank", "Team ID", "Name", "RP", "W", "T", "L", "Total Score");
    printf("============================================================================\n");

    for (int i = 0; i < count; i++) {
        Team *t = sorted[i];
        printf("%-6d %-8d %-20s %-5d %-5d %-5d %-5d %-10d\n",
               i + 1, t->id, t->name, t->ranking_points,
               t->wins, t->ties, t->losses, t->total_score);
    }
}

void ranking_free_sorted(Team **arr) {
    free(arr);
}
