#!/usr/bin/env bash
# C++ Systems Architecture – study a day, or run the full quality gate.
#
#   ./procpp.sh              menu: pick a day to study, or run the full check
#   ./procpp.sh 18           study Day 18: explanation, code map, notes, tests, quiz, bonus questions
#   ./procpp.sh 18 --quiz    only the quiz (2 multiple-choice questions) and the bonus questions
#   ./procpp.sh 18 --demo    …and run the day's demo straight away
#   ./procpp.sh --check      format check + warnings-as-errors build + regenerate docs + all tests (= CI)
#   ./procpp.sh --commit     full check, then commit and push the *current* branch
#
# With no arguments and no terminal (e.g. in a script), it runs --check.
set -euo pipefail
cd "$(dirname "$0")"

PYTHON="$(command -v python3 || command -v python)"
BUILD_DIR="${CPPM_BUILD_DIR:-build}"
JOBS="$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)"

configure() {
    local generator=()
    if command -v ninja >/dev/null 2>&1; then generator=(-G Ninja); fi
    cmake -S . -B "$BUILD_DIR" "${generator[@]}" -DCMAKE_BUILD_TYPE=Debug -DCPPM_WERROR=ON >/dev/null
}

full_check() {
    echo "------------------------------------------"
    echo "⚙️  C++ SYSTEMS ARCHITECTURE: ENGINEERING CHECK"
    echo "------------------------------------------"
    if command -v clang-format >/dev/null 2>&1; then
        echo "🎨 clang-format"
        find src include tests -name '*.hpp' -o -name '*.cpp' | xargs clang-format --dry-run --Werror
    else
        echo "🎨 clang-format not installed – skipping the format check"
    fi
    echo "🏗️  build (warnings are errors)"
    configure
    cmake --build "$BUILD_DIR" -j "$JOBS"
    echo "📝 reflections"; "$PYTHON" scripts/build_reflections.py
    echo "🧪 ctest"; ctest --test-dir "$BUILD_DIR" -j "$JOBS" --output-on-failure
    if ! git diff --quiet -- docs/progress README.md; then
        echo "⚠️  Reflections/README were regenerated – review and commit them."
    fi
    echo "✅ All checks passed."
}

commit_and_push() {
    branch="$(git rev-parse --abbrev-ref HEAD)"
    git status --short
    if git ls-files --others --exclude-standard | grep -qE '(^|/)\.env$'; then
        echo "❌ A .env file is about to be committed – aborting." >&2
        exit 1
    fi
    read -r -p "Commit message for '$branch': " message
    [[ -n "$message" ]] || { echo "Empty message – nothing committed."; exit 1; }
    git add -A
    git commit -m "$message"
    git push -u origin "$branch"
}

menu() {
    echo "------------------------------------------"
    echo "⚙️  C++ SYSTEMS ARCHITECTURE"
    echo "------------------------------------------"
    echo "  1–100   study that day (explanation, code map, notes, tests, quiz)"
    echo "  c       run the full quality check (same as CI)"
    echo "  q       quit"
    read -r -p "Your choice: " choice
    case "$choice" in
        q|Q|"") exit 0 ;;
        c|C) full_check ;;
        *[!0-9]*) echo "Please enter a day number, c or q." >&2; exit 2 ;;
        *) exec "$PYTHON" scripts/learn.py "$choice" ;;
    esac
}

case "${1:-}" in
    --check|--all) full_check ;;
    --commit) full_check; commit_and_push ;;
    -h|--help) sed -n '2,11p' "$0" | sed 's/^# \{0,1\}//' ;;
    "") if [[ -t 0 ]]; then menu; else full_check; fi ;;
    *[!0-9]*) echo "Unknown option '$1' – try ./procpp.sh --help" >&2; exit 2 ;;
    *) exec "$PYTHON" scripts/learn.py "$@" ;;
esac
