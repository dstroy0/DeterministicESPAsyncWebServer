#!/usr/bin/env bash
# run_tests.sh - Build, run, and document all PlatformIO native test suites.
#
# Usage:
#   ./test/run_tests.sh      # from project root
#   ./run_tests.sh           # from test/ directory
#
# Writes: docs/TEST_REPORT.md
#
# Requires: bash 4.2+, pio (PlatformIO Core), awk, sed, grep, mktemp

set -uo pipefail

# ── Paths ─────────────────────────────────────────────────────────────────────

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
if [[ "$(basename "$SCRIPT_DIR")" == "test" ]]; then
    PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
else
    PROJECT_ROOT="$SCRIPT_DIR"
fi
TEST_DIR="${PROJECT_ROOT}/test"
REPORT_PATH="${PROJECT_ROOT}/docs/TEST_REPORT.md"

# ── Find pio ──────────────────────────────────────────────────────────────────

PIO=""
for _candidate in \
    "pio" \
    "${HOME}/.platformio/penv/bin/pio" \
    "${HOME}/.local/bin/pio" \
    "/usr/local/bin/pio"; do
    if [[ -x "$_candidate" ]] 2>/dev/null || command -v "$_candidate" &>/dev/null 2>&1; then
        PIO="$_candidate"
        break
    fi
done

if [[ -z "$PIO" ]]; then
    echo "error: pio not found. Install PlatformIO Core or add it to PATH." >&2
    exit 1
fi

echo "pio     : $PIO"
echo "project : $PROJECT_ROOT"
echo "report  : $REPORT_PATH"
echo ""
echo "Running tests..."
echo ""

# Format output helper function
format_output() {
    awk -F'\t' '
    BEGIN {
        width = 80
    }
    {
        line = $0
        clean_line = line
        gsub(/\x1b\[[0-9;?]*[a-zA-Z]/, "", clean_line)
        
        # Test line: "test/test_X/test_X.cpp:829: test_name\t[PASSED]"
        if (clean_line ~ /\[(PASSED|FAILED)\]$/) {
            match(line, /[[:space:]]+(\x1b\[[0-9;?]*[a-zA-Z])*\[(PASSED|FAILED)\](\x1b\[[0-9;?]*[a-zA-Z])*$/)
            if (RSTART > 0) {
                left = substr(line, 1, RSTART - 1)
                right = substr(line, RSTART)
                sub(/^[[:space:]]+/, "", right)
                
                clean_left = left
                gsub(/\x1b\[[0-9;?]*[a-zA-Z]/, "", clean_left)
                
                clean_right = right
                gsub(/\x1b\[[0-9;?]*[a-zA-Z]/, "", clean_right)
                
                pad = width - length(clean_left) - length(clean_right)
                if (pad < 1) pad = 1
                spaces = ""
                for (i = 1; i <= pad; i++) spaces = spaces " "
                print left spaces right
                next
            }
        }
        
        # Banner line: "------------ native_ssh:test_ssh_crypto [PASSED] Took 6.70 seconds ------------"
        if (clean_line ~ /^-+ .* \[(PASSED|FAILED)\] Took .* seconds -+$/) {
            orig_msg = line
            gsub(/^-+/, "", orig_msg)
            gsub(/-+$/, "", orig_msg)
            gsub(/^[ \t]+|[ \t]+$/, "", orig_msg)
            
            clean_msg = orig_msg
            gsub(/\x1b\[[0-9;?]*[a-zA-Z]/, "", clean_msg)
            msg_len = length(clean_msg)
            
            pad_total = width - msg_len - 2
            if (pad_total >= 2) {
                pad_left = int(pad_total / 2)
                pad_right = pad_total - pad_left
                dashes_left = ""
                for (i = 1; i <= pad_left; i++) dashes_left = dashes_left "-"
                dashes_right = ""
                for (i = 1; i <= pad_right; i++) dashes_right = dashes_right "-"
                print dashes_left " " orig_msg " " dashes_right
                next
            }
        }
        
        # Section line: "--------------------------------------------------------------------------------"
        if (clean_line ~ /^-{20,}$/) {
            dashes = ""
            for (i = 1; i <= width; i++) dashes = dashes "-"
            print dashes
            next
        }
        
        print line
    }'
}

# ── Run tests ─────────────────────────────────────────────────────────────────

cd "$PROJECT_ROOT"

RAW_FILE="$(mktemp)"
CLEAN_FILE="$(mktemp)"
trap 'rm -f "$RAW_FILE" "$CLEAN_FILE"' EXIT

T0=$SECONDS
set +e
"$PIO" test -e native -e native_app -e native_ssh -e native_ssh_hardened -e native_ssh_conn -e native_compliance -e native_tsan -e native_workers 2>&1 | format_output | tee "$RAW_FILE"
PIO_EXIT="${PIPESTATUS[0]}"
set -e
WALL_SECS=$(( SECONDS - T0 ))

# Strip ANSI codes for parsing
sed 's/\x1b\[[0-9;?]*[a-zA-Z]//g' "$RAW_FILE" > "$CLEAN_FILE"

# ── Parse results ─────────────────────────────────────────────────────────────

# Parallel indexed arrays - bash 4.0+
T_IDX=0
declare -a T_ENV=() T_SUITE=() T_FILE=() T_LINE=() T_NAME=() T_STATUS=()

# Suite summary data
declare -A S_STATUS=() S_DURATION=() S_ENV=()
SUITE_ORDER=()   # insertion-order list of "env:suite" keys

CUR_ENV="native"
CUR_SUITE=""

while IFS= read -r line; do
    # Section header
    if [[ "$line" =~ ^Processing[[:space:]]+([^[:space:]]+)[[:space:]]+in[[:space:]]+([^[:space:]]+)[[:space:]]+environment ]]; then
        CUR_SUITE="${BASH_REMATCH[1]}"
        CUR_ENV="${BASH_REMATCH[2]}"
        continue
    fi

    # Individual test result
    if [[ "$line" =~ ([^[:space:]]+\.cpp):([0-9]+):[[:space:]]+([^[:space:]]+)[[:space:]]+\[(PASSED|FAILED)\] ]]; then
        rel_file="${BASH_REMATCH[1]//\\//}"
        suite="$(basename "${rel_file%.cpp}")"
        T_ENV[$T_IDX]="$CUR_ENV"
        T_SUITE[$T_IDX]="$suite"
        T_FILE[$T_IDX]="$rel_file"
        T_LINE[$T_IDX]="${BASH_REMATCH[2]}"
        T_NAME[$T_IDX]="${BASH_REMATCH[3]}"
        T_STATUS[$T_IDX]="${BASH_REMATCH[4]}"
        (( T_IDX++ )) || true
        continue
    fi

    # Suite summary line: "native  test_X  PASSED  00:00:02.07"
    if [[ "$line" =~ ^[[:space:]]*(native[^[:space:]]*)[[:space:]]+(test_[^[:space:]]+)[[:space:]]+(PASSED|FAILED)[[:space:]]+([0-9:\.]+) ]]; then
        _env="${BASH_REMATCH[1]}"
        _suite="${BASH_REMATCH[2]}"
        _key="${_env}:${_suite}"
        if [[ -z "${S_STATUS[$_key]:-}" ]]; then
            S_ENV[$_key]="$_env"
            S_STATUS[$_key]="${BASH_REMATCH[3]}"
            S_DURATION[$_key]="${BASH_REMATCH[4]}"
            SUITE_ORDER+=("$_key")
        fi
    fi
done < "$CLEAN_FILE"

# Totals
N_TOTAL=0; N_PASSED=0; N_FAILED=0
# Sum the per-invocation summary line(s): "N test cases: M succeeded in ...".
# The literal ": " anchor avoids a greedy ".*" mis-capturing the second count.
while IFS= read -r _line; do
    if [[ "$_line" =~ ([0-9]+)[[:space:]]+test[[:space:]]+cases:[[:space:]]+([0-9]+)[[:space:]]+succeeded ]]; then
        N_TOTAL=$(( N_TOTAL + BASH_REMATCH[1] ))
        N_PASSED=$(( N_PASSED + BASH_REMATCH[2] ))
    fi
done < <(grep -E '[0-9]+ test cases:' "$CLEAN_FILE")
N_FAILED=$(( N_TOTAL - N_PASSED ))

# ── Description helpers ───────────────────────────────────────────────────────

# get_test_comment <rel_file> <fn_name>
# Prints the first // comment inside the function body, or nothing.
get_test_comment() {
    local rel_file="$1"
    local fn_name="$2"
    local abs_file="${PROJECT_ROOT}/${rel_file}"
    [[ -f "$abs_file" ]] || return 0

    local fn_line
    fn_line=$(grep -n "void ${fn_name}(" "$abs_file" 2>/dev/null | head -1 | cut -d: -f1) || true
    [[ -n "$fn_line" ]] || return 0

    # Extract from function onwards; find first // comment after the opening brace
    tail -n "+${fn_line}" "$abs_file" | head -20 | awk '
        BEGIN { brace=0 }
        !brace && /\{/ { brace=1; next }
        brace && /^[[:space:]]*\/\// {
            sub(/^[[:space:]]*\/\/[[:space:]]*/, "")
            print; exit
        }
        brace && /TEST_ASSERT/ { exit }
        brace && /^[[:space:]]*[a-z_][a-zA-Z0-9_]*[[:space:]]*[=(]/ { exit }
    '
}

# get_suite_brief <suite_name>
# Returns the first meaningful description line from the file-level comment block.
get_suite_brief() {
    local suite="$1"
    local abs_file="${PROJECT_ROOT}/test/${suite}/${suite}.cpp"
    [[ -f "$abs_file" ]] || return 0

    awk '
        /^\/\/ (Copyright|SPDX)/ { past_header=1; next }
        past_header && /^\/\/$/ { next }
        past_header && /^\/\/ (.+)/ {
            sub(/^\/\/ /, ""); print; exit
        }
        /^#include/ { exit }
    ' "$abs_file"
}

# name_to_desc <fn_name>
# Converts snake_case test/stress/race name to a readable sentence.
name_to_desc() {
    local name="$1"
    local prefix=""

    if [[ "$name" == stress_* ]]; then
        prefix="Stress - "
        name="${name#stress_}"
    elif [[ "$name" == race_* ]]; then
        prefix="Race - "
        name="${name#race_}"
    elif [[ "$name" == test_* ]]; then
        name="${name#test_}"
    fi

    # Replace underscores with spaces; capitalize first character
    name="${name//_/ }"
    printf '%s%s' "$prefix" "${name^}"
}

# suite_count <suite> <env> <status_filter>
# status_filter: "PASSED", "FAILED", or "" for all
suite_count() {
    local suite="$1" env="$2" filter="$3" cnt=0
    for (( i=0; i<T_IDX; i++ )); do
        [[ "${T_SUITE[$i]}" == "$suite" && "${T_ENV[$i]}" == "$env" ]] || continue
        [[ -z "$filter" || "${T_STATUS[$i]}" == "$filter" ]] || continue
        (( cnt++ )) || true
    done
    echo $cnt
}

# ── Build report ──────────────────────────────────────────────────────────────

OV_ICON="✅"; [[ $N_FAILED -gt 0 ]] && OV_ICON="❌"
DATE_STR="$(date '+%Y-%m-%d %H:%M:%S')"
RES_STR="${OV_ICON} ${N_PASSED} passed"
[[ $N_FAILED -gt 0 ]] && RES_STR+=", ${N_FAILED} failed"
RES_STR+=" - ${WALL_SECS}s"

{
cat <<EOF
# Test Report

**Generated:** ${DATE_STR}
**Command:** \`pio test -e native -e native_app -e native_ssh -e native_ssh_hardened -e native_ssh_conn -e native_compliance -e native_tsan -e native_workers\`
**Result:** ${RES_STR}

---

## Summary

| Suite | Environment | Tests | Status | Duration |
| :--- | :--- | ---: | :---: | ---: |
EOF

for _key in "${SUITE_ORDER[@]}"; do
    _suite="${_key#*:}"
    _env="${S_ENV[$_key]}"
    _cnt=$(suite_count "$_suite" "$_env" "")
    _st="${S_STATUS[$_key]}"
    _dur="${S_DURATION[$_key]}"
    _icon="✅"; [[ "$_st" == "FAILED" ]] && _icon="❌"
    printf '| `%s` | `%s` | %s | %s | %s |\n' \
        "$_suite" "$_env" "$_cnt" "$_icon" "$_dur"
done

printf '\n---\n\n'

# Per-suite detail sections
declare -A _DONE=()
for (( i=0; i<T_IDX; i++ )); do
    _suite="${T_SUITE[$i]}"
    _env="${T_ENV[$i]}"
    _key="${_env}:${_suite}"
    [[ -n "${_DONE[$_key]:-}" ]] && continue
    _DONE[$_key]=1

    _pass=$(suite_count "$_suite" "$_env" "PASSED")
    _fail=$(suite_count "$_suite" "$_env" "FAILED")
    _gicon="✅"; [[ $_fail -gt 0 ]] && _gicon="❌"
    _brief=$(get_suite_brief "$_suite") || true

    _header="## ${_suite} - ${_gicon} ${_pass} passed"
    [[ $_fail -gt 0 ]] && _header+=", ${_fail} failed"
    echo "$_header"
    echo ""
    echo "<details>"
    echo "<summary><b>Expand Suite Details</b></summary>"
    echo ""
    [[ -n "$_brief" ]] && printf '*%s*\n\n' "$_brief"

    echo "| # | Test | Status | Description |"
    echo "|---:|:---|:---:|:---|"

    _n=1
    for (( j=0; j<T_IDX; j++ )); do
        [[ "${T_SUITE[$j]}" != "$_suite" || "${T_ENV[$j]}" != "$_env" ]] && continue
        _name="${T_NAME[$j]}"
        _status="${T_STATUS[$j]}"
        _ticon="✅"; [[ "$_status" == "FAILED" ]] && _ticon="❌ **FAILED**"

        _desc=$(get_test_comment "${T_FILE[$j]}" "$_name") || true
        [[ -z "$_desc" ]] && _desc=$(name_to_desc "$_name")

        printf '| %d | `%s` | %s | %s |\n' "$_n" "$_name" "$_ticon" "$_desc"
        (( _n++ )) || true
    done
    echo ""

    if [[ $_fail -gt 0 ]]; then
        echo "### Failure detail"
        echo ""
        echo '```'
        grep '\[FAILED\]' "$CLEAN_FILE" | grep "$_suite" || true
        echo '```'
        echo ""
    fi
    echo "</details>"
    echo ""
    echo "---"
    echo ""
done

# Collapsible raw output
cat <<'RAWEOF'
## Raw Output

<details>
<summary>Expand full pio output</summary>

```
RAWEOF
cat "$CLEAN_FILE"
printf '```\n\n</details>\n'

} > "$REPORT_PATH"

echo ""
echo "Report written: $REPORT_PATH"
exit $PIO_EXIT
