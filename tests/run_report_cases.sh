#!/bin/sh
set -eu

BIN=${1:-./tournament}
ROOT=${2:-$(pwd)}

cleanup_dirs=""
pass_count=0

print_case_output() {
    id=$1
    title=$2
    output=$3

    printf '\n===== %s - %s =====\n' "$id" "$title"
    printf '%s\n' "$output"
}

cleanup() {
    for dir in $cleanup_dirs; do
        rm -rf "$dir"
    done
}

trap cleanup EXIT INT TERM

copy_fixture_file() {
    rel_path=$1
    dest=$2

    if git -C "$ROOT" cat-file -e "HEAD:$rel_path" 2>/dev/null; then
        git -C "$ROOT" show "HEAD:$rel_path" > "$dest"
    elif [ -f "$ROOT/$rel_path" ]; then
        cp "$ROOT/$rel_path" "$dest"
    fi
}

make_fixture_dir() {
    mode=$1
    dir=$(mktemp -d)
    cleanup_dirs="${cleanup_dirs} ${dir}"
    mkdir -p "$dir/data"

    case "$mode" in
        full)
            copy_fixture_file "data/teams.csv" "$dir/data/teams.csv"
            copy_fixture_file "data/matches.csv" "$dir/data/matches.csv"
            ;;
        no_teams)
            copy_fixture_file "data/matches.csv" "$dir/data/matches.csv"
            ;;
        no_matches)
            copy_fixture_file "data/teams.csv" "$dir/data/teams.csv"
            ;;
        bad_teams_row)
            copy_fixture_file "data/matches.csv" "$dir/data/matches.csv"
            cat > "$dir/data/teams.csv" <<'EOF'
team_id,team_name,rp,wins,ties,losses,total_score
1001,Red Dragons,0,0,0,1,70
bad,row
1002,Blue Thunder,0,0,0,0,0
EOF
            ;;
        bad_matches_row)
            copy_fixture_file "data/teams.csv" "$dir/data/teams.csv"
            cat > "$dir/data/matches.csv" <<'EOF'
match_num,red_team_id,blue_team_id,red_score,blue_score,status
1,1001,1002,100,56,1
bad,row
2,1003,1001,0,0,0
EOF
            ;;
        *)
            printf 'Unknown fixture mode: %s\n' "$mode" >&2
            exit 1
            ;;
    esac

    : > "$dir/data/results.csv"
    printf '%s\n' "$dir"
}

assert_patterns() {
    output=$1
    shift

    for pattern do
        if ! printf '%s' "$output" | grep -Eq "$pattern"; then
            printf 'Missing expected pattern: %s\n' "$pattern" >&2
            printf '%s\n' "$output" >&2
            exit 1
        fi
    done
}

run_case() {
    id=$1
    title=$2
    mode=$3
    input=$4
    shift 4

    case_dir=$(make_fixture_dir "$mode")
    output_file=$(mktemp)

    (
        cd "$case_dir"
        printf '%b' "$input" | "$BIN"
    ) > "$output_file"

    output=$(cat "$output_file")
    rm -f "$output_file"
    rm -rf "$case_dir"

    assert_patterns "$output" "$@"

    pass_count=$((pass_count + 1))
    printf '[PASS] %s - %s\n' "$id" "$title"
    print_case_output "$id" "$title" "$output"
}

run_case \
    "TC-01" \
    "Import Teams" \
    "full" \
    '1\n7\n' \
    'Loaded 10 teams\.'

run_case \
    "TC-02" \
    "Import Teams Missing File" \
    "no_teams" \
    '1\n7\n' \
    'Warning: Cannot open data/teams\.csv' \
    'Failed to load teams\. Keeping existing data\.'

run_case \
    "TC-03" \
    "Import Teams Malformed Row" \
    "bad_teams_row" \
    '1\n7\n' \
    'Warning: Skipping malformed row: bad,row' \
    'Loaded 2 teams\.'

run_case \
    "TC-04" \
    "Import Schedule" \
    "full" \
    '2\n7\n' \
    'Loaded 20 matches \(3 pending\)\.'

run_case \
    "TC-05" \
    "Import Schedule Missing File" \
    "no_matches" \
    '2\n7\n' \
    'Warning: Cannot open data/matches\.csv' \
    'Failed to load matches\. Keeping existing data\.'

run_case \
    "TC-06" \
    "Import Schedule Malformed Row" \
    "bad_matches_row" \
    '2\n7\n' \
    'Warning: Skipping malformed row: bad,row' \
    'Loaded 2 matches \(1 pending\)\.'

run_case \
    "TC-07" \
    "Invalid Main Menu Input" \
    "full" \
    'abc\n7\n' \
    'Invalid input\. Please enter a number\.'

run_case \
    "TC-08" \
    "Play Match Without Teams" \
    "full" \
    '3\n7\n' \
    'No teams loaded\. Please import teams first \(option 1\)\.'

run_case \
    "TC-09" \
    "Play Match Without Schedule" \
    "no_matches" \
    '1\n3\n7\n' \
    'No matches loaded\. Please import the schedule first \(option 2\)\.'

run_case \
    "TC-10" \
    "Invalid Score Input" \
    "full" \
    '1\n2\n3\na\n7\n' \
    'Invalid score input\. Match not recorded\.'

printf 'Report cases passed: %d\n' "$pass_count"
