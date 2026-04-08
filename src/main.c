#include "../include/tournament.h"

void print_menu(void) {
    printf("\n========================================\n");
    printf("   ROBOTICS TOURNAMENT MANAGEMENT SYSTEM\n");
    printf("========================================\n");
    printf(" [1] Import Teams from CSV\n");
    printf(" [2] Import Match Schedule from CSV\n");
    printf(" [3] Play Next Match (Enter Scores)\n");
    printf(" [4] View Match Schedule & Results\n");
    printf(" [5] View Rankings\n");
    printf(" [6] Export Rankings to CSV\n");
    printf(" [7] Exit\n");
    printf("========================================\n");
    printf("Select option: ");
}

int main(void) {
    Team *teams = NULL;
    Match *matches = NULL;
    MatchQueue queue;
    int team_count = 0;
    int match_count = 0;
    int choice;

    queue_init(&queue);

    printf("Welcome to Robotics Tournament Manager!\n");

    while (1) {
        print_menu();
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            printf("Invalid input. Please enter a number.\n");
            continue;
        }

        switch (choice) {
            case 1:
                teams = file_load_teams(TEAMS_FILE, &team_count);
                printf("Loaded %d teams.\n", team_count);
                break;
            case 2:
                matches = file_load_matches(MATCHES_FILE, &match_count);
                queue_build_pending(&queue, matches);
                printf("Loaded %d matches (%d pending).\n", match_count, queue.size);
                break;
            case 3: {
                if (!teams) {
                    printf("No teams loaded. Please import teams first (option 1).\n");
                    break;
                }
                Match *m = queue_dequeue(&queue);
                if (!m) {
                    printf("All matches have been played.\n");
                } else {
                    int red_score, blue_score;
                    printf("Match %d: ", m->match_num);
                    match_print_one(m, teams);
                    printf("Enter RED score: ");
                    scanf("%d", &red_score);
                    printf("Enter BLUE score: ");
                    scanf("%d", &blue_score);
                    match_enter_score(m, teams, red_score, blue_score);
                    printf("Score recorded.\n");
                }
                break;
            }
            case 4:
                match_print_all(matches, teams);
                break;
            case 5: {
                Team **sorted = ranking_sort(teams, &team_count);
                ranking_display(sorted, team_count);
                ranking_free_sorted(sorted);
                break;
            }
            case 6: {
                Team **sorted = ranking_sort(teams, &team_count);
                file_save_rankings(sorted, team_count, RANKINGS_FILE);
                ranking_free_sorted(sorted);
                printf("Rankings exported to %s\n", RANKINGS_FILE);
                break;
            }
            case 7:
                file_save_teams(teams, TEAMS_FILE);
                file_save_matches(matches, MATCHES_FILE);
                team_free_list(teams);
                match_free_list(matches);
                queue_free(&queue);
                printf("Goodbye!\n");
                return 0;
            default:
                printf("Invalid option.\n");
        }
    }
}
