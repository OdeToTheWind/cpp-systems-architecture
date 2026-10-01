"""Study one day of the course in the terminal.

Usage::

    python3 scripts/learn.py            # asks for a day number
    python3 scripts/learn.py 18         # study Day 18
    python3 scripts/learn.py 18 --demo  # …and run its demo at the end
    python3 scripts/learn.py 18 --no-tests
    python3 scripts/learn.py 18 --quiz  # only the questions

For the chosen day it prints, in this order:

1. what the day is about (topic, level, phase, scenario),
2. what you will learn (syllabus deliverables) and **where each one lives in the code**,
3. the key learnings and pitfalls from the day's notes,
4. what the tests prove, then builds and runs them so you see them pass,
5. two multiple-choice questions, open bonus questions, and the next step.

Everything comes from the repository itself – syllabus.md, the day's lesson.hpp, its tests and
``docs/progress/notes/day-XX.json`` – so it is always in step with the code.
"""

from __future__ import annotations

import argparse
import contextlib
import json
import os
import shutil
import subprocess
import sys
import textwrap
from collections.abc import Callable
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))

import build_reflections as br  # noqa: E402

WIDTH = min(100, shutil.get_terminal_size((100, 20)).columns)
BUILD_DIR = Path(os.environ.get("CPPM_BUILD_DIR", ROOT / "build"))


class Style:
    """ANSI colours only on a real terminal, and never when NO_COLOR is set."""

    def __init__(self, enabled: bool) -> None:
        self.enabled = enabled

    def _wrap(self, code: str, text: str) -> str:
        return f"\033[{code}m{text}\033[0m" if self.enabled else text

    def title(self, text: str) -> str:
        return self._wrap("1;36", text)

    def heading(self, text: str) -> str:
        return self._wrap("1;33", text)

    def dim(self, text: str) -> str:
        return self._wrap("2", text)

    def code(self, text: str) -> str:
        return self._wrap("32", text)


def wrap(text: str, indent: str = "  ") -> str:
    """Wrap to the terminal width; continuation lines line up under the text, not the bullet."""
    return textwrap.fill(" ".join(text.split()), WIDTH, initial_indent=indent,
                         subsequent_indent=" " * len(indent))


def wrap_exact(text: str, indent: str = "  ") -> str:
    """Like :func:`wrap` but keeps runs of spaces – quiz options can depend on them."""
    return textwrap.fill(text, WIDTH, initial_indent=indent, subsequent_indent=" " * len(indent),
                         break_on_hyphens=False)


def phase_of(day: int) -> str:
    for number, name, first, last in br.phases():
        if first <= day <= last:
            return f"Phase {number} · {name} (Days {first}–{last})"
    return "–"


def explain(day: int, style: Style) -> None:
    row = br.syllabus_rows()[day]
    day_notes = br.notes(day)
    rule = "─" * WIDTH

    print(style.title(rule))
    print(style.title(f"  Day {day} · {row['topic']}"))
    print(style.dim(f"  {row['level']} · {phase_of(day)}"))
    print(style.title(rule))

    print(style.heading("\n▸ The scenario"))
    print(wrap(br.lesson_scenario(day)))

    print(style.heading("\n▸ What you will learn"))
    print(wrap(row["deliverables"]))

    print(style.heading("\n▸ Where each skill lives in the code"))
    for skill, symbol in br.deliverables(day).items():
        location = br.locate(day, symbol)
        print(f"  • {skill}")
        if location is None:
            print(f"      {style.code(symbol)} (not found – run the tooling tests)")
            continue
        print(f"      {style.code(location.where())}")
        summary = location.declaration + (f" — {location.summary}" if location.summary else "")
        print(textwrap.fill(summary, WIDTH, initial_indent="      ", subsequent_indent="        "))

    print(style.heading("\n▸ Key learnings"))
    for item in day_notes["learnings"]:  # type: ignore[union-attr]
        print(wrap(item, "  • "))
    print(style.heading("\n▸ Pitfalls to avoid"))
    for item in day_notes["pitfalls"]:  # type: ignore[union-attr]
        print(wrap(item, "  ⚠ "))

    names = br.test_names(day)
    print(style.heading(f"\n▸ What the {len(names)} tests prove"))
    for name in names:
        print(f"  ✓ {name}")


def _ensure_built(target: str, style: Style) -> Path | None:
    """Configure (once) and build *target*; return the executable path, or None on failure."""
    cmake = shutil.which("cmake")
    if cmake is None:
        print("  cmake was not found on PATH – install CMake 3.20+ to build and run the code.", file=sys.stderr)
        return None
    if not (BUILD_DIR / "CMakeCache.txt").exists():
        print(style.dim(f"  configuring {BUILD_DIR.name}/ (first run only)…"))
        configure = [cmake, "-S", str(ROOT), "-B", str(BUILD_DIR)]
        if shutil.which("ninja"):
            configure += ["-G", "Ninja"]
        if subprocess.run(configure, cwd=ROOT, check=False, stdout=subprocess.DEVNULL).returncode != 0:
            return None
    build = subprocess.run([cmake, "--build", str(BUILD_DIR), "--target", target], cwd=ROOT, check=False,
                           stdout=subprocess.DEVNULL)
    if build.returncode != 0:
        print(f"  building {target} failed – run: cmake --build {BUILD_DIR.name} --target {target}",
              file=sys.stderr)
        return None
    for candidate in (BUILD_DIR / "bin" / target, BUILD_DIR / "bin" / f"{target}.exe",
                      BUILD_DIR / "bin" / "Debug" / f"{target}.exe"):
        if candidate.exists():
            return candidate
    return None


def run_tests(day: int, style: Style) -> int:
    target = f"test_day_{day:02d}"
    print(style.heading(f"\n▸ Building and running tests/{target}.cpp"))
    executable = _ensure_built(target, style)
    if executable is None:
        return 1
    return subprocess.run([str(executable)], cwd=ROOT, check=False).returncode


def run_demo(day: int, style: Style) -> int:
    target = br.day_dir(day).name
    print(style.heading(f"\n▸ Demo: build/bin/{target}"))
    executable = _ensure_built(target, style)
    if executable is None:
        return 1
    return subprocess.run([str(executable)], cwd=ROOT, check=False).returncode


def next_steps(day: int, style: Style) -> None:
    lesson = br.rel(br.lesson_path(day))
    day_notes = br.notes(day)
    print(style.heading("\n▸ Try it yourself"))
    print(f"  1. Open {style.code(lesson)} and change one line of a function listed above.")
    print(f"  2. Re-run {style.code(f'./procpp.sh {day}')} – watch a test turn red and read why.")
    print("  3. Undo the change (or fix it properly) until everything is green again.")
    print(f"  4. Read the full reflection: {style.code(f'docs/progress/day-{day:02d}-reflection.md')}")
    print(style.heading("\n▸ Next step"))
    print(wrap(str(day_notes["next"])))
    print()


QUIZ_DIR = ROOT / "docs" / "quiz"


def load_quiz(day: int) -> dict[str, Any]:
    return json.loads((QUIZ_DIR / f"day-{day:02d}.json").read_text(encoding="utf-8"))


def run_quiz(day: int, style: Style, ask: Callable[[str], str] | None = None) -> tuple[int, int]:
    """Ask the day's multiple-choice questions; return ``(correct, asked)``.

    Answers are typed as a letter (A–D). Without a terminal the questions are only listed,
    so an answer key is never printed.
    """
    questions = load_quiz(day)["mcq"]
    print(style.heading(f"\n▸ Quick check: {len(questions)} multiple-choice questions"))
    if ask is None and not sys.stdin.isatty():
        for number, item in enumerate(questions, 1):
            print(wrap_exact(f"Q{number}. {item['question']}"))
            for letter, option in item["options"].items():
                print(wrap_exact(f"{letter}) {option}", "      "))
        print(style.dim("  (Run ./procpp.sh in a terminal to answer them.)"))
        return 0, 0
    ask = ask or input
    correct = 0
    for number, item in enumerate(questions, 1):
        print()
        print(wrap_exact(f"Q{number}. {item['question']}"))
        for letter, option in item["options"].items():
            print(wrap_exact(f"{letter}) {option}", "      "))
        while True:
            try:
                answer = ask("  Your answer (A–D): ").strip().upper()
            except EOFError:
                return correct, number - 1
            if answer in item["options"]:
                break
            print("  Please type one letter: A, B, C or D.")
        if answer == item["answer"]:
            correct += 1
            print(style.code("  ✔ Correct!"))
        else:
            print(wrap_exact(f"✘ Not quite – the answer is {item['answer']}) {item['options'][item['answer']]}"))
        print(wrap_exact(item["explanation"], "    "))
    print(style.heading(f"\n  Score: {correct}/{len(questions)}"))
    return correct, len(questions)


def show_bonus(day: int, style: Style) -> None:
    """Open questions with no answer key: research them, or turn them into code and tests."""
    bonus = load_quiz(day)["bonus"]
    print(style.heading(f"\n▸ Bonus questions ({len(bonus)}) – no answers given, find out for yourself"))
    for number, item in enumerate(bonus, 1):
        tag = "🧪 Hands-on" if item["type"] == "hands-on" else "💬 Think & explain"
        print(wrap(f"{number}. [{tag}] {item['question']}"))


def ask_day(prompt: str = "Which day do you want to study? (1–100, q to quit): ") -> int | None:
    while True:
        try:
            answer = input(prompt).strip().lower()
        except EOFError:
            return None
        if answer in {"q", "quit", "exit", ""}:
            return None
        if answer.isdigit() and 1 <= int(answer) <= 100:
            return int(answer)
        print("  Please enter a whole number from 1 to 100.")


def ask_yes(prompt: str) -> bool:
    try:
        return input(prompt).strip().lower() in {"y", "yes"}
    except EOFError:
        return False


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(prog="learn", description="Study one day: explanation, code map and tests.")
    parser.add_argument("day", nargs="?", type=int, help="day number 1–100 (asked for when omitted)")
    parser.add_argument("--demo", action="store_true", help="run the day's demo without asking")
    parser.add_argument("--no-tests", action="store_true", help="explain only, do not build or run the tests")
    parser.add_argument("--no-quiz", action="store_true", help="skip the multiple-choice and bonus questions")
    parser.add_argument("--quiz", action="store_true", help="only the questions: skip explanation, tests and demo")
    args = parser.parse_args(argv)

    with contextlib.suppress(AttributeError, ValueError):  # emoji on Windows consoles
        sys.stdout.reconfigure(encoding="utf-8")  # type: ignore[union-attr]
    interactive = sys.stdin.isatty()
    style = Style(sys.stdout.isatty() and "NO_COLOR" not in os.environ)

    day = args.day if args.day is not None else ask_day()
    if day is None:
        return 0
    rows = br.syllabus_rows()
    if day not in rows:
        print(f"There is no Day {day}. Choose a number from 1 to 100.", file=sys.stderr)
        return 2
    if rows[day]["status"] != "Covered":
        print(f"Day {day} is planned but not written yet.", file=sys.stderr)
        return 2

    if args.quiz:
        run_quiz(day, style)
        show_bonus(day, style)
        return 0
    explain(day, style)
    code = 0 if args.no_tests else run_tests(day, style)
    if args.demo or (interactive and ask_yes("\nRun the demo now? [y/N] ")):
        run_demo(day, style)
    if not args.no_quiz:
        run_quiz(day, style)
        show_bonus(day, style)
    next_steps(day, style)
    return code


if __name__ == "__main__":
    sys.exit(main())
