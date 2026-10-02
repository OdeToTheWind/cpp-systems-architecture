"""Tests for scripts/learn.py – the per-day study mode behind ./procpp.sh <day>."""

from __future__ import annotations

import contextlib
import io
import sys
import unittest
from pathlib import Path
from unittest import mock

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "scripts"))

import build_reflections as br  # noqa: E402
import learn  # noqa: E402

PLAIN = learn.Style(enabled=False)
COVERED = sorted(d for d, row in br.syllabus_rows().items() if row["status"] == "Covered")


def captured(function, *args):
    out, err = io.StringIO(), io.StringIO()
    with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
        result = function(*args)
    return result, out.getvalue(), err.getvalue()


class Explain(unittest.TestCase):
    def test_every_covered_day_can_be_explained(self) -> None:
        for day in COVERED:
            with self.subTest(day=day):
                _, out, _ = captured(learn.explain, day, PLAIN)
                self.assertIn(f"Day {day} · ", out)
                self.assertIn("▸ Where each skill lives in the code", out)
                self.assertIn(f"src/day_{day:02d}_", out)
                self.assertNotIn("(not found – run the tooling tests)", out)
                self.assertIn("▸ Pitfalls to avoid", out)
                self.assertIn("tests prove", out)

    def test_explain_shows_phase_and_test_names(self) -> None:
        _, out, _ = captured(learn.explain, 5, PLAIN)
        self.assertIn("Phase 1 · Beginner Fundamentals (Days 1–24)", out)
        for name in br.test_names(5):
            self.assertIn(f"✓ {name}", out)


class CommandLine(unittest.TestCase):
    def test_unknown_day_is_rejected(self) -> None:
        result, _, err = captured(learn.main, ["101", "--no-tests", "--no-quiz"])
        self.assertEqual(result, 2)
        self.assertIn("There is no Day 101", err)

    def test_explain_only_mode_does_not_build(self) -> None:
        with mock.patch.object(learn, "run_tests") as run_tests, \
                mock.patch.object(learn.sys.stdin, "isatty", return_value=False):
            result, out, _ = captured(learn.main, ["2", "--no-tests", "--no-quiz"])
        self.assertEqual(result, 0)
        run_tests.assert_not_called()
        self.assertIn("▸ Next step", out)

    def test_ask_day_accepts_numbers_and_quits_on_q(self) -> None:
        with mock.patch("builtins.input", side_effect=["abc", "0", "42"]):
            _, out, _ = captured(learn.ask_day)
        self.assertIn("Please enter a whole number", out)
        with mock.patch("builtins.input", side_effect=["q"]):
            self.assertIsNone(learn.ask_day())

    def test_style_adds_colour_only_when_enabled(self) -> None:
        self.assertEqual(PLAIN.heading("x"), "x")
        self.assertIn("\033[", learn.Style(enabled=True).heading("x"))


if __name__ == "__main__":
    unittest.main()
