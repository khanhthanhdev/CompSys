#include "../include/tournament.h"
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>

static void fail(const char *message) {
    fprintf(stderr, "TEST FAIL: %s\n", message);
    exit(1);
}

static void check(int condition, const char *message) {
    if (!condition) {
        fail(message);
    }
}

static Team *append_team(Team **head, Team **tail, int id, const char *name) {
    Team *t = team_create(id, name);
    check(t != NULL, "team_create returned NULL");

    if (!*head) {
        *head = t;
    } else {
        (*tail)->next = t;
    }
    *tail = t;
    return t;
}

static Match *append_match(Match **head, Match **tail, int num, int red_id, int blue_id,
                           int red_score, int blue_score, MatchStatus status) {
    Match *m = match_create(num, red_id, blue_id);
    check(m != NULL, "match_create returned NULL");
    m->red_score = red_score;
    m->blue_score = blue_score;
    m->status = status;

    if (!*head) {
        *head = m;
    } else {
        (*tail)->next = m;
    }
    *tail = m;
    return m;
}

static void write_text_file(const char *path, const char *content) {
    FILE *f = fopen(path, "w");
    check(f != NULL, "failed to open file for writing");
    check(fputs(content, f) >= 0, "failed to write file content");
    fclose(f);
}

static long count_file_lines(const char *path) {
    FILE *f = fopen(path, "r");
    check(f != NULL, "failed to open file for reading");

    long lines = 0;
    char buffer[MAX_LINE_LEN];
    while (fgets(buffer, sizeof(buffer), f)) {
        lines++;
    }

    fclose(f);
    return lines;
}

static void make_temp_dir(char *path, size_t size) {
    snprintf(path, size, "/tmp/rms_project_test_%ld", (long)getpid());
    if (mkdir(path, 0700) != 0) {
        fail("failed to create temp directory");
    }
}

static void pushd(const char *path, char *oldcwd, size_t size) {
    check(getcwd(oldcwd, size) != NULL, "getcwd failed");
    check(chdir(path) == 0, "chdir failed");
}

static void popd(const char *oldcwd) {
    check(chdir(oldcwd) == 0, "restore cwd failed");
}

static void test_team_stats_helpers(void) {
    Team *t = team_create(1001, "Alpha");
    check(t != NULL, "team_create failed");

    team_update_stats(t, 10, 5);
    check(t->matches_played == 1, "win should increment matches_played");
    check(t->wins == 1, "win should increment wins");
    check(t->ranking_points == RP_WIN, "win should add RP");
    check(t->total_score == 10, "win should add score");

    team_update_stats(t, 7, 7);
    check(t->ties == 1, "tie should increment ties");
    check(t->ranking_points == RP_WIN + RP_TIE, "tie should add RP");
    check(t->total_score == 17, "tie should add score");

    team_update_stats(t, 2, 9);
    check(t->losses == 1, "loss should increment losses");
    check(t->ranking_points == RP_WIN + RP_TIE, "loss should not add RP");
    check(t->total_score == 19, "loss should add score");

    team_reset_stats(t);
    check(t->ranking_points == 0, "reset should clear ranking points");
    check(t->matches_played == 0, "reset should clear matches_played");
    check(t->wins == 0 && t->ties == 0 && t->losses == 0, "reset should clear result counters");
    check(t->total_score == 0, "reset should clear total_score");

    team_free_list(t);
}

static void test_rebuild_stats_from_matches(void) {
    Team *teams = NULL;
    Team *tail = NULL;
    append_team(&teams, &tail, 1, "Alpha");
    append_team(&teams, &tail, 2, "Beta");
    append_team(&teams, &tail, 3, "Gamma");

    Match *matches = NULL;
    Match *match_tail = NULL;
    append_match(&matches, &match_tail, 1, 1, 2, 5, 3, MATCH_PLAYED);
    append_match(&matches, &match_tail, 2, 2, 3, 0, 0, MATCH_PENDING);
    append_match(&matches, &match_tail, 3, 1, 3, 4, 4, MATCH_PLAYED);

    team_rebuild_stats_from_matches(teams, matches);

    Team *alpha = team_find(teams, 1);
    Team *beta = team_find(teams, 2);
    Team *gamma = team_find(teams, 3);

    check(alpha && beta && gamma, "team_find failed");
    check(alpha->matches_played == 2, "alpha should have 2 played matches");
    check(alpha->wins == 1 && alpha->ties == 1 && alpha->losses == 0, "alpha results mismatch");
    check(alpha->ranking_points == 4, "alpha RP mismatch");
    check(alpha->total_score == 9, "alpha score mismatch");

    check(beta->matches_played == 1, "beta should have 1 played match");
    check(beta->wins == 0 && beta->ties == 0 && beta->losses == 1, "beta results mismatch");
    check(beta->ranking_points == 0, "beta RP mismatch");
    check(beta->total_score == 3, "beta score mismatch");

    check(gamma->matches_played == 1, "gamma should have 1 played match");
    check(gamma->wins == 0 && gamma->ties == 1 && gamma->losses == 0, "gamma results mismatch");
    check(gamma->ranking_points == 1, "gamma RP mismatch");
    check(gamma->total_score == 4, "gamma score mismatch");

    team_free_list(teams);
    match_free_list(matches);
}

static void test_ranking_sort(void) {
    Team *teams = NULL;
    Team *tail = NULL;
    Team *t1 = append_team(&teams, &tail, 20, "T20");
    Team *t2 = append_team(&teams, &tail, 10, "T10");
    Team *t3 = append_team(&teams, &tail, 30, "T30");
    Team *t4 = append_team(&teams, &tail, 5, "T05");

    t1->ranking_points = 6;
    t1->total_score = 30;
    t2->ranking_points = 6;
    t2->total_score = 30;
    t3->ranking_points = 9;
    t3->total_score = 5;
    t4->ranking_points = 6;
    t4->total_score = 40;

    int count = 0;
    Team **sorted = ranking_sort(teams, &count);
    check(sorted != NULL, "ranking_sort returned NULL");
    check(count == 4, "ranking_sort count mismatch");
    check(sorted[0]->id == 30, "highest RP should rank first");
    check(sorted[1]->id == 5, "higher score should break RP tie");
    check(sorted[2]->id == 10, "lower team ID should break remaining tie");
    check(sorted[3]->id == 20, "remaining team order mismatch");

    ranking_free_sorted(sorted);
    team_free_list(teams);
}

static void test_queue_build_pending(void) {
    Match *matches = NULL;
    Match *tail = NULL;
    append_match(&matches, &tail, 1, 1, 2, 10, 5, MATCH_PLAYED);
    append_match(&matches, &tail, 2, 2, 3, 0, 0, MATCH_PENDING);
    append_match(&matches, &tail, 3, 3, 4, 7, 4, MATCH_PLAYED);
    append_match(&matches, &tail, 4, 4, 1, 0, 0, MATCH_PENDING);

    MatchQueue queue;
    queue_init(&queue);
    queue_build_pending(&queue, matches);

    check(queue.size == 2, "queue size mismatch");
    check(queue_peek(&queue)->match_num == 2, "first pending match mismatch");
    check(queue_dequeue(&queue)->match_num == 2, "queue order mismatch");
    check(queue_dequeue(&queue)->match_num == 4, "queue order mismatch");
    check(queue_dequeue(&queue) == NULL, "queue should now be empty");
    check(queue.size == 0, "queue size should be zero");

    queue_free(&queue);
    match_free_list(matches);
}

static void test_file_loaders_and_match_entry(void) {
    char original_cwd[PATH_MAX];
    char temp_dir[64];
    char data_dir[80];
    char teams_path[96];
    char matches_path[96];
    char results_path[96];

    make_temp_dir(temp_dir, sizeof(temp_dir));
    snprintf(data_dir, sizeof(data_dir), "%s/data", temp_dir);
    check(mkdir(data_dir, 0700) == 0, "failed to create data directory");

    snprintf(teams_path, sizeof(teams_path), "%s/teams.csv", data_dir);
    snprintf(matches_path, sizeof(matches_path), "%s/matches.csv", data_dir);
    snprintf(results_path, sizeof(results_path), "%s/results.csv", data_dir);

    write_text_file(teams_path,
        "team_id,team_name,rp,wins,ties,losses,total_score\n"
        "1,Alpha,0,0,0,0,0\n"
        "2,Beta,0,0,0,0,0\n");
    write_text_file(matches_path,
        "match_num,red_team_id,blue_team_id,red_score,blue_score,status\n"
        "1,1,2,30,20,1\n"
        "2,2,1,0,0,0\n");

    pushd(temp_dir, original_cwd, sizeof(original_cwd));

    int team_count = 0;
    int match_count = 0;
    Team *teams = file_load_teams(TEAMS_FILE, &team_count);
    Match *matches = file_load_matches(MATCHES_FILE, &match_count);

    check(teams != NULL, "file_load_teams returned NULL");
    check(matches != NULL, "file_load_matches returned NULL");
    check(team_count == 2, "team count mismatch");
    check(match_count == 2, "match count mismatch");

    team_rebuild_stats_from_matches(teams, matches);

    Team *alpha = team_find(teams, 1);
    Team *beta = team_find(teams, 2);
    Match *pending = match_find(matches, 2);
    check(alpha && beta && pending, "lookup failed after load");
    check(alpha->ranking_points == 3 && alpha->wins == 1 && alpha->total_score == 30, "alpha stats mismatch after rebuild");
    check(beta->losses == 1 && beta->total_score == 20, "beta stats mismatch after rebuild");
    check(pending->status == MATCH_PENDING, "second match should start pending");

    match_enter_score(pending, teams, 18, 25);
    check(pending->status == MATCH_PLAYED, "match should be marked played");
    check(alpha->ranking_points == 6 && alpha->wins == 2, "alpha stats should update after play");
    check(beta->ranking_points == 0 && beta->losses == 2, "beta stats should update after play");
    check(count_file_lines(RESULTS_FILE) == 1, "results file should contain one recorded result");

    int alpha_rp = alpha->ranking_points;
    int beta_rp = beta->ranking_points;
    match_enter_score(pending, teams, 99, 1);
    check(alpha->ranking_points == alpha_rp, "played match should not be scored twice");
    check(beta->ranking_points == beta_rp, "played match should not be scored twice");
    check(count_file_lines(RESULTS_FILE) == 1, "results file should not gain a second row");

    team_free_list(teams);
    match_free_list(matches);
    popd(original_cwd);
}

int main(void) {
    test_team_stats_helpers();
    test_rebuild_stats_from_matches();
    test_ranking_sort();
    test_queue_build_pending();
    test_file_loaders_and_match_entry();

    puts("All core tests passed.");
    return 0;
}
