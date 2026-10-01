"""Keep syllabus.md, src/, tests/ and docs/progress/ in lock-step.

A day may only be marked **Covered** when its code, tests and reflection all exist and agree.
A **Planned** day must not have code yet (otherwise the status is understated). Reflections and
the README's generated blocks must equal what ``scripts/build_reflections.py`` would generate now.
"""

from __future__ import annotations

import re
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "scripts"))

import build_reflections as br  # noqa: E402

ROWS = br.syllabus_rows()
COVERED = sorted(day for day, row in ROWS.items() if row["status"] == "Covered")
PLANNED = sorted(day for day, row in ROWS.items() if row["status"] == "Planned")
PLACEHOLDER = re.compile(r"\b(CHECK|REQUIRE)\s*\(\s*(true|1)\s*\)")


class SyllabusShape(unittest.TestCase):
    def test_syllabus_has_100_unique_days_with_known_statuses(self) -> None:
        self.assertEqual(sorted(ROWS), list(range(1, 101)))
        self.assertLessEqual({row["status"] for row in ROWS.values()}, {"Covered", "Planned"})
        self.assertEqual(list(range(1, len(COVERED) + 1)), COVERED, "days must be covered in order")

    def test_levels_are_known(self) -> None:
        self.assertLessEqual({row["level"] for row in ROWS.values()},
                             {"Beginner", "Intermediate", "Advanced", "Capstone"})

    def test_syllabus_phases_cover_every_day_once(self) -> None:
        days = [d for _n, _name, first, last in br.phases() for d in range(first, last + 1)]
        self.assertEqual(days, list(range(1, 101)), "phase headings must cover days 1–100 in order")


class CoveredDays(unittest.TestCase):
    def test_covered_day_has_lesson_with_resolvable_deliverables(self) -> None:
        for day in COVERED:
            with self.subTest(day=day):
                folders = list((ROOT / "src").glob(f"day_{day:02d}_*"))
                self.assertEqual(len(folders), 1, f"expected exactly one src folder for day {day}")
                folder = folders[0]
                self.assertTrue((folder / "lesson.hpp").exists(), "every day needs lesson.hpp")
                self.assertTrue(br.lesson_scenario(day), "lesson.hpp must open with a Scenario: paragraph")
                table = br.deliverables(day)
                self.assertGreaterEqual(len(table), 3, "map at least three skills to code")
                for skill, symbol in table.items():
                    self.assertIsNotNone(br.locate(day, symbol), f"{skill!r} points to missing {symbol!r}")
                main = (folder / "main.cpp").read_text(encoding="utf-8")
                self.assertIn('#include "lesson.hpp"', main, "main.cpp must be a thin demo shell")
                self.assertIn("run(", main, "main.cpp must call the lesson's run() demo")

    def test_covered_day_tests_exercise_src(self) -> None:
        for day in COVERED:
            with self.subTest(day=day):
                folder = br.day_dir(day)
                source = br.test_path(day).read_text(encoding="utf-8")
                self.assertIn(f'#include "{folder.name}/lesson.hpp"', source, "tests must include the day's own code")
                names = br.test_names(day)
                self.assertGreaterEqual(len(names), 5, "each day needs a meaningful set of tests")
                self.assertEqual(len(names), len(set(names)), "test names must be unique within a day")
                self.assertIsNone(PLACEHOLDER.search(source), "placeholder assertions are not allowed")
                self.assertIn("run(", source, "tests must also drive the demo with scripted input")

    def test_notes_are_complete(self) -> None:
        for day in COVERED:
            with self.subTest(day=day):
                notes = br.notes(day)
                self.assertRegex(str(notes["date"]), r"^\d{4}-\d{2}-\d{2}$")
                self.assertGreaterEqual(len(notes["learnings"]), 3)  # type: ignore[arg-type]
                self.assertGreaterEqual(len(notes["pitfalls"]), 2)  # type: ignore[arg-type]
                self.assertTrue(notes["next"])

    def test_reflection_is_generated_from_current_code(self) -> None:
        for day in COVERED:
            with self.subTest(day=day):
                notes = br.notes(day)
                expected = br.render(day, ROWS[day], notes, str(notes["date"]))
                actual = (ROOT / "docs" / "progress" / f"day-{day:02d}-reflection.md").read_text(encoding="utf-8")
                self.assertEqual(actual, expected, "reflection is stale – run: python3 scripts/build_reflections.py")

    def test_scenarios_are_unique_per_day(self) -> None:
        scenarios = [br.lesson_scenario(day) for day in COVERED]
        duplicates = {s for s in scenarios if scenarios.count(s) > 1}
        self.assertFalse(duplicates, f"each day needs its own scenario: {duplicates}")

    def test_no_stray_progress_files(self) -> None:
        allowed = {f"day-{d:02d}-reflection.md" for d in COVERED} | {"day-XX-reflection.md"}
        actual = {p.name for p in (ROOT / "docs" / "progress").glob("*.md")}
        self.assertEqual(actual - allowed, set(), "only generated reflections belong in docs/progress/")


class PlannedDays(unittest.TestCase):
    def test_planned_day_has_no_code_yet(self) -> None:
        for day in PLANNED:
            with self.subTest(day=day):
                self.assertFalse(list((ROOT / "src").glob(f"day_{day:02d}_*")),
                                 f"day {day} has code – mark it Covered in syllabus.md")
                self.assertFalse(br.test_path(day).exists())


class Readme(unittest.TestCase):
    def test_readme_generated_blocks_are_current(self) -> None:
        readme = (ROOT / "README.md").read_text(encoding="utf-8")
        for name in br.README_BLOCKS:
            self.assertIn(f"<!-- {name}:start -->", readme, f"README lost its generated {name!r} block")
        self.assertEqual(readme, br.render_readme(readme, ROWS),
                         "README generated blocks are stale – run: python3 scripts/build_reflections.py")


class Locator(unittest.TestCase):
    def test_locate_reports_file_line_and_declaration(self) -> None:
        day = COVERED[0]
        symbol = next(iter(br.deliverables(day).values()))
        location = br.locate(day, symbol)
        assert location is not None
        self.assertRegex(location.where(), rf"^src/day_{day:02d}_\w+/.+:\d+$")
        self.assertIn(symbol.split("::")[-1], location.declaration)

    def test_locate_returns_none_for_unknown_symbols(self) -> None:
        self.assertIsNone(br.locate(COVERED[0], "definitely_not_defined_anywhere"))


if __name__ == "__main__":
    unittest.main()
