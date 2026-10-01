"""Regenerate every Covered day's reflection, and the README's generated tables, from the source of truth.

Usage: ``python3 scripts/build_reflections.py``

For each Covered day it reads:

* the syllabus row (topic, deliverables, level),
* the ``Scenario:`` paragraph and the ``DELIVERABLES`` table in ``src/day_XX_<topic>/lesson.hpp``,
* the ``TEST_CASE`` names in ``tests/test_day_XX.cpp``,
* hand-written notes from ``docs/progress/notes/day-XX.json`` (learnings, pitfalls, next step),

and writes ``docs/progress/day-XX-reflection.md``. ``tests/tooling/test_syllabus_sync.py`` checks
that the generated files are current and that every deliverable symbol is really defined.
"""

from __future__ import annotations

import json
import re
import sys
from dataclasses import dataclass
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

ROW = re.compile(r"^\|\s*(\d+)\s*\|\s*(.+?)\s*\|\s*(.+?)\s*\|\s*(\w+)\s*\|\s*(\w+)\s*\|$")


def syllabus_rows() -> dict[int, dict[str, str]]:
    """``{day: {topic, deliverables, level, status}}`` parsed from syllabus.md."""
    rows: dict[int, dict[str, str]] = {}
    for line in (ROOT / "syllabus.md").read_text(encoding="utf-8").splitlines():
        match = ROW.match(line)
        if match:
            day, topic, deliverables, level, status = match.groups()
            rows[int(day)] = {
                "topic": topic,
                "deliverables": deliverables.replace("\\|", "|"),
                "level": level,
                "status": status,
            }
    return rows


def day_dir(day: int) -> Path:
    (path,) = (ROOT / "src").glob(f"day_{day:02d}_*")
    return path


def lesson_path(day: int) -> Path:
    return day_dir(day) / "lesson.hpp"


def test_path(day: int) -> Path:
    return ROOT / "tests" / f"test_day_{day:02d}.cpp"


def rel(path: Path) -> str:
    return path.relative_to(ROOT).as_posix()


# ── Reading a lesson ──────────────────────────────────────────────────────────

def file_doc(text: str) -> str:
    """The first ``/** ... */`` comment with the leading `` * `` of each line removed."""
    match = re.search(r"/\*\*(.*?)\*/", text, re.S)
    if not match:
        return ""
    lines = [re.sub(r"^\s*\*\s?", "", line) for line in match.group(1).splitlines()]
    return "\n".join(lines)


def scenario(doc: str) -> str:
    """The ``Scenario:`` paragraph of a lesson's file comment, as one sentence-cased line."""
    match = re.search(r"Scenario:\s*(.+?)(?:\n\s*\n|$)", doc, re.S)
    text = " ".join(match.group(1).split()) if match else ""
    return text[:1].upper() + text[1:]


def lesson_scenario(day: int) -> str:
    return scenario(file_doc(lesson_path(day).read_text(encoding="utf-8")))


DELIVERABLE_ENTRY = re.compile(r'\{\s*"((?:[^"\\]|\\.)*)"\s*,\s*"([^"]+)"\s*\}')


def deliverables(day: int) -> dict[str, str]:
    """``{skill: symbol}`` from the lesson's ``DELIVERABLES`` table, in declaration order."""
    text = lesson_path(day).read_text(encoding="utf-8")
    match = re.search(r"\bDELIVERABLES\b[^=]*=\s*\{(.*?)\};", text, re.S)
    if not match:
        return {}
    return {skill.replace('\\"', '"'): symbol for skill, symbol in DELIVERABLE_ENTRY.findall(match.group(1))}


@dataclass(frozen=True)
class Location:
    """Where a deliverable symbol is defined: file, line, the declaration and its doc summary."""

    path: Path
    line: int
    declaration: str
    summary: str

    def where(self) -> str:
        return f"{rel(self.path)}:{self.line}"


STATEMENT_KEYWORDS = re.compile(r"^\s*(return|if|else|while|for|switch|case|throw|co_return|co_yield|delete|new)\b")


def _definition_patterns(name: str, constructor: bool) -> list[re.Pattern[str]]:
    escaped = re.escape(name)
    prefix = r"(?:[\w:<>,*&\[\]~\s]*?[\w>*&\]]\s+)" + ("?" if constructor else "")
    return [
        re.compile(rf"^\s*(?:template\s*<.*>\s*)?(?:inline\s+)?(?:class|struct|union|enum(?:\s+class)?|concept|namespace)"
                   rf"\s+(?:\w+::)*{escaped}\b"),
        re.compile(rf"^\s*(?:template\s*<.*>\s*)?using\s+{escaped}\s*="),
        re.compile(rf"^\s*#\s*define\s+{escaped}\b"),
        re.compile(rf"^\s*(?:template\s*<.*>\s*)?(?:\[\[\w+\]\]\s*)?{prefix}(?:\w+::)*~?{escaped}\s*\("),
        re.compile(rf"^\s*(?:inline\s+|static\s+|extern\s+|thread_local\s+)*(?:constexpr|const|constinit)\b[^=(;]*?\b{escaped}\b\s*(?:=|\{{|\[)"),
        re.compile(rf"^\s*(?:inline\s+|static\s+)*(?:const\s+)?[\w:<>,\s*&]+[\s*&]{escaped}\s*(?:=|\{{|;)"),
    ]


def _is_code(line: str) -> bool:
    stripped = line.strip()
    return bool(stripped) and not stripped.startswith(("//", "*", "/*"))


def _doc_above(lines: list[str], index: int) -> str:
    """First sentence of the ``///`` or ``/** */`` comment directly above line *index*."""
    collected: list[str] = []
    i = index - 1
    while i >= 0 and re.match(r"^\s*(template\s*<|\[\[)", lines[i]):
        i -= 1
    while i >= 0:
        stripped = lines[i].strip()
        if stripped.startswith("///"):
            collected.insert(0, stripped[3:].strip())
        elif stripped.endswith("*/") or stripped.startswith("*") or stripped.startswith("/**"):
            text = re.sub(r"^/\*\*|\*/$|^\*", "", stripped).strip()
            if text:
                collected.insert(0, text)
            if stripped.startswith("/**"):
                break
        else:
            break
        i -= 1
    text = " ".join(part for part in collected if not part.startswith("@"))
    sentence = re.split(r"(?<=[.!?])\s", text, maxsplit=1)[0]
    return sentence.strip()


def _source_files(day: int) -> list[Path]:
    folder = day_dir(day)
    files = sorted(p for p in folder.rglob("*") if p.suffix in {".hpp", ".h", ".cpp"})
    files.sort(key=lambda p: (p.name != "lesson.hpp", p.suffix != ".hpp", p.as_posix()))
    return files


def locate(day: int, symbol: str) -> Location | None:
    """Find the definition (or declaration) of *symbol* (``name`` or ``Scope::name``) in the day's code."""
    parts = symbol.split("::")
    name = parts[-1]
    constructor = len(parts) > 1 and parts[-1] == parts[-2]
    patterns = _definition_patterns(name, constructor)
    scope_patterns = _definition_patterns(parts[-2], False)[:1] if len(parts) > 1 else []
    for path in _source_files(day):
        lines = path.read_text(encoding="utf-8").splitlines()
        start = 0
        if scope_patterns:
            scope_line = next((i for i, ln in enumerate(lines) if scope_patterns[0].search(ln)), None)
            if scope_line is None:
                qualified = re.compile(rf"(?:^|[\s*&]){re.escape(parts[-2])}::{re.escape(name)}\s*\(")
                hit = next((i for i, ln in enumerate(lines) if _is_code(ln) and qualified.search(ln)
                            and not STATEMENT_KEYWORDS.match(ln)), None)
                if hit is not None:
                    return Location(path, hit + 1, _clean(lines[hit]), _doc_above(lines, hit))
                continue
            start = scope_line  # the scope line itself may hold the name (`namespace units::metric`)
        for i in range(start, len(lines)):
            line = lines[i]
            if not _is_code(line) or STATEMENT_KEYWORDS.match(line):
                continue
            if any(pattern.search(line) for pattern in patterns):
                return Location(path, i + 1, _clean(line), _doc_above(lines, i))
    return None


def _clean(line: str) -> str:
    text = " ".join(line.split())
    return re.sub(r"\s*\{\s*$", "", text).rstrip(";")


# ── Reading tests and notes ───────────────────────────────────────────────────

TEST_CASE = re.compile(r'^TEST_CASE\("((?:[^"\\]|\\.)*)"\)', re.M)


def test_names(day: int) -> list[str]:
    return TEST_CASE.findall(test_path(day).read_text(encoding="utf-8"))


def count_tests(day: int) -> int:
    return len(test_names(day))


def notes(day: int) -> dict[str, object]:
    path = ROOT / "docs" / "progress" / "notes" / f"day-{day:02d}.json"
    return json.loads(path.read_text(encoding="utf-8"))


# ── Rendering ─────────────────────────────────────────────────────────────────

def _escape(text: str) -> str:
    """Stop Markdown from turning ``*ptr`` or ``__func__`` into emphasis."""
    return text.replace("\\", "\\\\").replace("*", "\\*").replace("_", "\\_")


def render(day: int, row: dict[str, str], day_notes: dict[str, object], date: str) -> str:
    folder = day_dir(day)
    lesson = rel(lesson_path(day))
    test = rel(test_path(day))
    table = "\n".join(f"| ✅ {_escape(skill)} | `{symbol}` |" for skill, symbol in deliverables(day).items())
    learnings = "\n".join(f"- {item}" for item in day_notes["learnings"])  # type: ignore[union-attr]
    pitfalls = "\n".join(f"- {item}" for item in day_notes["pitfalls"])  # type: ignore[union-attr]
    return f"""# Day {day:02d} – {row['topic']} Reflection

**Date:** {date} · **Level:** {row['level']} · **C++:** 20 · **Status:** {row['status']}
**Code:** [`{lesson}`](../../{lesson}) · **Tests:** [`{test}`](../../{test}) ({count_tests(day)} tests)

## Scenario
{lesson_scenario(day)}

## Syllabus deliverables
> {_escape(row['deliverables'])}

| Deliverable | Implemented in |
|---|---|
{table}

## Key learnings
{learnings}

## Pitfalls I hit (and how I fixed them)
{pitfalls}

## Run it
```bash
./procpp.sh {day}                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/{folder.name}       # the interactive demo
ctest --test-dir build -R test_day_{day:02d} --output-on-failure
```

## Next step
- {day_notes['next']}
"""


LEVEL_BADGE = {"Beginner": "🟢", "Intermediate": "🟡", "Advanced": "🟠", "Capstone": "🔴"}


def course_index(rows: dict[int, dict[str, str]]) -> str:
    """Markdown table for the README: one line per day, linking code, tests and reflection."""
    lines = ["| Day | Topic | Level | Scenario you build | Links |", "|---:|---|:-:|---|---|"]
    for day, row in sorted(rows.items()):
        badge = LEVEL_BADGE.get(row["level"], "")
        if row["status"] != "Covered":
            lines.append(f"| {day} | {row['topic']} | {badge} | _planned_ | – |")
            continue
        links = (f"[code]({rel(lesson_path(day))}) · [tests]({rel(test_path(day))}) · "
                 f"[notes](docs/progress/day-{day:02d}-reflection.md)")
        lines.append(f"| {day} | {row['topic']} | {badge} | {lesson_scenario(day)} | {links} |")
    return "\n".join(lines)


PHASE = re.compile(r"^## Phase (\d+) · (.+?) \(Days (\d+)–(\d+)\)$")


def phases() -> list[tuple[int, str, int, int]]:
    """``(number, name, first_day, last_day)`` from the ``## Phase …`` headings in syllabus.md."""
    found = []
    for line in (ROOT / "syllabus.md").read_text(encoding="utf-8").splitlines():
        match = PHASE.match(line)
        if match:
            number, name, first, last = match.groups()
            found.append((int(number), name, int(first), int(last)))
    return found


FOCUS = re.compile(r"^\| (\d+) · .+? \| \d+–\d+ \| (.+) \|$")


def topic_map(rows: dict[int, dict[str, str]]) -> str:
    """Three-column map of the content: phase, its days, and what those days cover."""
    focus = {}
    for line in (ROOT / "syllabus.md").read_text(encoding="utf-8").splitlines():
        match = FOCUS.match(line)
        if match:
            focus[int(match.group(1))] = match.group(2)
    lines = ["| Phase | Days | What it covers |", "|---|:-:|---|"]
    for number, name, first, last in phases():
        anchor = f"phase-{number}--{name}-days-{first}{last}".lower()
        anchor = re.sub(r"[^a-z0-9 -]", "", anchor.replace(" ", "-"))
        lines.append(f"| [{number} · {name}](syllabus.md#{anchor}) | {first}–{last} | {focus.get(number, '')} |")
    return "\n".join(lines)


WORKFLOW = ROOT / ".github" / "workflows" / "cpp-tests.yml"
OS_NAMES = {"ubuntu": "Linux", "windows": "Windows", "macos": "macOS"}


def source_lines() -> int:
    return sum(len([ln for ln in f.read_text(encoding="utf-8").splitlines() if ln.strip()])
               for f in (ROOT / "src").rglob("*") if f.suffix in {".hpp", ".cpp", ".h"})


def kpis(rows: dict[int, dict[str, str]]) -> str:
    """Numbers measured from the repository itself, so the README can never overstate them."""
    covered = [d for d, r in rows.items() if r["status"] == "Covered"]
    tests = sum(count_tests(d) for d in covered)
    mapped = sum(len(deliverables(d)) for d in covered)
    workflow = WORKFLOW.read_text(encoding="utf-8") if WORKFLOW.exists() else ""
    compilers = re.findall(r"compiler-name:\s*\"?([^\"\n]+?)\"?\s*$", workflow, re.M)
    gate = re.search(r"--fail-under-line[= ](\d+)", workflow)
    systems = sorted(set(re.findall(r"(ubuntu|windows|macos)-[\w.]+", workflow)))
    return "\n".join([
        f"- **Curriculum completion:** {len(covered)} / {len(rows)} days covered, each with code, tests and a reflection.",
        f"- **Test cases:** {tests} `TEST_CASE`s across {len(covered)} test executables.",
        f"- **Deliverables mapped to code:** {mapped} `DELIVERABLES` entries, each checked to resolve to a definition.",
        f"- **Source size:** {source_lines():,} non-blank lines of C++ in `src/`.",
        quiz_kpi(covered),
        f"- **Coverage gate:** CI fails below {gate.group(1) if gate else '?'} % line coverage of `src/`.",
        f"- **Compilers in CI:** {', '.join(dict.fromkeys(compilers)) or '?'}.",
        f"- **Operating systems in CI:** {', '.join(OS_NAMES[s] for s in systems) or '?'}.",
        "- **Quality checks per commit:** warnings as errors · clang-format · ASan + UBSan · unit tests with coverage · syllabus sync.",
    ])


def quiz_kpi(covered: list[int]) -> str:
    quizzes = [json.loads((ROOT / "docs" / "quiz" / f"day-{d:02d}.json").read_text(encoding="utf-8"))
               for d in covered if (ROOT / "docs" / "quiz" / f"day-{d:02d}.json").exists()]
    mcq = sum(len(q["mcq"]) for q in quizzes)
    bonus = [b for q in quizzes for b in q["bonus"]]
    hands_on = sum(b["type"] == "hands-on" for b in bonus)
    return (f"- **Self-check questions:** {mcq} multiple-choice questions with explanations, plus "
            f"{len(bonus)} open bonus questions ({hands_on} hands-on, test-first tasks).")


README_BLOCKS = {
    "course-index": course_index,
    "topic-map": topic_map,
    "kpis": kpis,
}


def render_readme(text: str, rows: dict[int, dict[str, str]]) -> str:
    for name, build in README_BLOCKS.items():
        start, end = f"<!-- {name}:start -->", f"<!-- {name}:end -->"
        if start not in text:
            continue
        before, rest = text.split(start, 1)
        _, after = rest.split(end, 1)
        text = f"{before}{start}\n{build(rows)}\n{end}{after}"
    return text


def update_readme(rows: dict[int, dict[str, str]]) -> None:
    readme = ROOT / "README.md"
    readme.write_text(render_readme(readme.read_text(encoding="utf-8"), rows), encoding="utf-8", newline="\n")


def main() -> int:
    rows = syllabus_rows()
    written = 0
    for day, row in sorted(rows.items()):
        if row["status"] != "Covered":
            continue
        day_notes = notes(day)
        target = ROOT / "docs" / "progress" / f"day-{day:02d}-reflection.md"
        target.write_text(render(day, row, day_notes, str(day_notes["date"])), encoding="utf-8", newline="\n")
        written += 1
    update_readme(rows)
    print(f"wrote {written} reflections and refreshed the README's generated sections")
    return 0


if __name__ == "__main__":
    sys.exit(main())
