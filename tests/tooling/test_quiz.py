"""Quality gate for the per-day quizzes in docs/quiz/ and the quiz mode of scripts/learn.py."""

from __future__ import annotations

import collections
import contextlib
import io
import json
import sys
import unittest
from pathlib import Path
from unittest import mock

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "scripts"))

import build_reflections as br  # noqa: E402
import learn  # noqa: E402

COVERED = sorted(d for d, row in br.syllabus_rows().items() if row["status"] == "Covered")
QUIZZES = {day: json.loads((ROOT / "docs" / "quiz" / f"day-{day:02d}.json").read_text(encoding="utf-8"))
           for day in COVERED}
ALL_MCQ = [q for quiz in QUIZZES.values() for q in quiz["mcq"]]
PLAIN = learn.Style(enabled=False)


def scripted(*answers: str):
    replies = iter(answers)

    def ask(_prompt: str) -> str:
        try:
            return next(replies)
        except StopIteration:
            raise EOFError from None

    return ask


def captured(function, *args):
    buffer = io.StringIO()
    with contextlib.redirect_stdout(buffer):
        result = function(*args)
    return result, buffer.getvalue()


class QuizContent(unittest.TestCase):
    def test_each_day_has_two_well_formed_mcqs(self) -> None:
        for day, quiz in QUIZZES.items():
            with self.subTest(day=day):
                self.assertEqual(quiz["day"], day)
                self.assertEqual(len(quiz["mcq"]), 2)
                for item in quiz["mcq"]:
                    self.assertEqual(list(item["options"]), ["A", "B", "C", "D"])
                    self.assertEqual(len(set(item["options"].values())), 4, "options must be distinct")
                    self.assertIn(item["answer"], item["options"])
                    self.assertTrue(item["question"].strip())
                    self.assertGreater(len(item["explanation"]), 40)
                    self.assertNotIn("option a", item["explanation"].lower(),
                                     "explanations must not depend on option order")

    def test_each_day_has_open_bonus_questions(self) -> None:
        for day, quiz in QUIZZES.items():
            with self.subTest(day=day):
                bonus = quiz["bonus"]
                self.assertGreaterEqual(len(bonus), 2)
                self.assertEqual({item["type"] for item in bonus}, {"discuss", "hands-on"})
                self.assertTrue(all(set(item) == {"type", "question"} for item in bonus),
                                "bonus questions have no answer key")

    def test_questions_are_unique_across_the_course(self) -> None:
        questions = [q["question"] for q in ALL_MCQ]
        self.assertEqual(len(questions), len(set(questions)))

    def test_answer_letters_are_balanced(self) -> None:
        counts = collections.Counter(q["answer"] for q in ALL_MCQ)
        for letter in "ABCD":
            self.assertTrue(0.2 <= counts[letter] / len(ALL_MCQ) <= 0.3, counts)
        pairs = collections.Counter(tuple(q["answer"] for q in quiz["mcq"]) for quiz in QUIZZES.values())
        self.assertGreaterEqual(len(pairs), 8, "the second answer must not follow a guessable pattern")


class QuizMode(unittest.TestCase):
    def test_quiz_scores_correct_answers(self) -> None:
        key = [q["answer"] for q in QUIZZES[1]["mcq"]]
        result, out = captured(learn.run_quiz, 1, PLAIN, scripted(key[0].lower(), key[1]))
        self.assertEqual(result, (2, 2))
        self.assertEqual(out.count("✔ Correct!"), 2)

    def test_quiz_explains_wrong_answers_and_reasks_on_bad_input(self) -> None:
        first = QUIZZES[5]["mcq"][0]
        wrong = next(letter for letter in "ABCD" if letter != first["answer"])
        second = QUIZZES[5]["mcq"][1]["answer"]
        result, out = captured(learn.run_quiz, 5, PLAIN, scripted("z", "", wrong, second))
        self.assertEqual(result, (1, 2))
        self.assertEqual(out.count("Please type one letter"), 2)
        self.assertIn(f"the answer is {first['answer']})", out)
        self.assertIn(first["explanation"][:30], out)

    def test_quiz_stops_cleanly_at_end_of_input(self) -> None:
        result, _ = captured(learn.run_quiz, 7, PLAIN, scripted())
        self.assertEqual(result, (0, 0))

    def test_quiz_without_terminal_lists_questions_but_never_the_answers(self) -> None:
        with mock.patch.object(learn.sys.stdin, "isatty", return_value=False):
            result, out = captured(learn.run_quiz, 9, PLAIN)
        self.assertEqual(result, (0, 0))
        self.assertIn(QUIZZES[9]["mcq"][0]["question"][:40], out)
        self.assertNotIn("Correct", out)
        self.assertNotIn(QUIZZES[9]["mcq"][0]["explanation"][:40], out)

    def test_bonus_questions_are_shown_without_answers(self) -> None:
        _, out = captured(learn.show_bonus, 12, PLAIN)
        self.assertIn("🧪 Hands-on", out)
        self.assertIn("💬 Think & explain", out)
        self.assertIn("no answers given", out)

    def test_quiz_only_mode_skips_explanation_and_tests(self) -> None:
        with mock.patch.object(learn.sys.stdin, "isatty", return_value=False):
            result, out = captured(learn.main, ["3", "--quiz"])
        self.assertEqual(result, 0)
        self.assertIn("Quick check", out)
        self.assertIn("Bonus questions", out)
        self.assertNotIn("Where each skill lives", out)


if __name__ == "__main__":
    unittest.main()
