#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="build"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

if command -v ninja >/dev/null 2>&1; then
    cmake .. -G Ninja
    ninja
else
    cmake .. -G "MinGW Makefiles"
    mingw32-make -j4
fi

echo -e "\nBuild finished."

# List built executables (relative from project root)
echo "Available executables:"
for exe in ../build/day_*.exe; do
    if [[ -f "$exe" ]]; then
        echo "  ./$exe"
    fi
done

if [[ -z "$(ls ../build/day_*.exe 2>/dev/null)" ]]; then
    echo "  (no day executables found yet)"
fi

echo ""
echo "Example run: ./build/day_02_strings.exe"

# Optional: git commit & push (comment out if you want to control manually)
# git add .
# git commit -m "Auto: $(date '+%Y-%m-%d %H:%M') - after build" || echo "Nothing new to commit"
# git push origin main || echo "Push skipped"