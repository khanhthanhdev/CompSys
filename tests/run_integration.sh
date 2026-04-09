#!/bin/sh
set -eu

BIN=${1:-./tournament}
ROOT=${2:-$(pwd)}
cleanup_dirs=""

cleanup() {
    for dir in $cleanup_dirs; do
        rm -rf "$dir"
    done
}

trap cleanup EXIT INT TERM

make_fixture_dir() {
    dir=$(mktemp -d)
    cleanup_dirs="${cleanup_dirs} ${dir}"
    mkdir -p "$dir/data"
    git -C "$ROOT" show HEAD:data/teams.csv > "$dir/data/teams.csv"
    git -C "$ROOT" show HEAD:data/matches.csv > "$dir/data/matches.csv"
    git -C "$ROOT" show HEAD:data/results.csv > "$dir/data/results.csv"
    printf '%s\n' "$dir"
}

run_case() {
    name=$1
    input=$2
    pattern=$3

    case_dir=$(make_fixture_dir)
    output_file=$(mktemp)
    (
        cd "$case_dir"
        printf '%b' "$input" | "$BIN"
    ) > "$output_file"
    output=$(cat "$output_file")
    rm -f "$output_file"
    rm -rf "$case_dir"
    printf '%s' "$output" | grep -Eq "$pattern" || {
        printf 'integration test failed: %s\n' "$name" >&2
        printf '%s\n' "$output" >&2
        exit 1
    }
}

count_case() {
    name=$1
    input=$2
    pattern=$3
    expected=$4

    case_dir=$(make_fixture_dir)
    output_file=$(mktemp)
    (
        cd "$case_dir"
        printf '%b' "$input" | "$BIN"
    ) > "$output_file"
    output=$(cat "$output_file")
    rm -f "$output_file"
    rm -rf "$case_dir"
    count=$(printf '%s' "$output" | grep -Ec "$pattern" || true)
    if [ "$count" -ne "$expected" ]; then
        printf 'integration test failed: %s\n' "$name" >&2
        printf 'expected %s matches for pattern %s, got %s\n' "$expected" "$pattern" "$count" >&2
        printf '%s\n' "$output" >&2
        exit 1
    fi
}

run_case \
    "teams then matches" \
    '1\n2\n5\n7\n' \
    '^1[[:space:]]+1001[[:space:]]+Red Dragons[[:space:]]+12[[:space:]]+4[[:space:]]+0[[:space:]]+1[[:space:]]+468'

run_case \
    "matches then teams" \
    '2\n1\n5\n7\n' \
    '^1[[:space:]]+1001[[:space:]]+Red Dragons[[:space:]]+12[[:space:]]+4[[:space:]]+0[[:space:]]+1[[:space:]]+468'

run_case \
    "invalid score input" \
    '1\n2\n3\na\nb\n4\n7\n' \
    'Invalid score input\. Match not recorded\.'

run_case \
    "invalid score leaves match pending" \
    '1\n2\n3\na\nb\n4\n7\n' \
    '^3[[:space:]]+Blue Thunder[[:space:]]+Green Force[[:space:]]+\|[[:space:]]+\?[[:space:]]+-[[:space:]]+\?[[:space:]]+\| PENDING$'

count_case \
    "repeat team import" \
    '1\n1\n7\n' \
    'Loaded 10 teams\.' \
    2

count_case \
    "repeat match import" \
    '1\n2\n2\n7\n' \
    'Loaded 20 matches \(3 pending\)\.' \
    2

run_case \
    "record played match" \
    '1\n2\n3\n10\n20\n5\n7\n' \
    'Score recorded\.'

run_case \
    "play next updates standings" \
    '1\n2\n3\n10\n20\n4\n7\n' \
    'Total: 20 matches \(18 played, 2 pending\)'

printf 'Integration tests passed.\n'
