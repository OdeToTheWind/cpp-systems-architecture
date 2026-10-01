# Day 47 – Desktop GUI Architecture Reflection

**Date:** 2026-04-29 · **Level:** Intermediate · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_47_gui_architecture/lesson.hpp`](../../src/day_47_gui_architecture/lesson.hpp) · **Tests:** [`tests/test_day_47.cpp`](../../tests/test_day_47.cpp) (7 tests)

## Scenario
A *travel currency converter* desktop app built with Model-View-Presenter. The model knows exchange rates, the view is a thin interface any toolkit (Qt, wxWidgets, a console) can implement, and the presenter holds every UI rule – so the whole app is tested headlessly with a fake view, no window needed.

## Syllabus deliverables
> Model-View-Presenter, widget-free presenters, input validation, headless testing of UI logic

| Deliverable | Implemented in |
|---|---|
| ✅ the model: data and rules, no UI | `RatesModel` |
| ✅ the view: a passive interface any toolkit can implement | `ConverterView` |
| ✅ the presenter: every UI rule, no widgets | `ConverterPresenter` |
| ✅ validating text typed into a field | `parse_amount` |
| ✅ a console view as one concrete implementation | `ConsoleView` |

## Key learnings
- Model-View-Presenter puts data in the model, keeps the view passive, and gives every UI rule to the presenter, which knows no widgets.
- Because the view is an interface, a fake view in the tests can record what the presenter asked it to show – the whole UI logic is tested headlessly.
- A Qt, wxWidgets or console front end only has to implement the view interface; the presenter and model do not change.
- Validating input on every change (enabling or disabling the Convert button) gives immediate feedback without dialogs.

## Pitfalls I hit (and how I fixed them)
- The first version parsed the amount inside a button-click handler tied to a widget class, so nothing could be tested without a window.
- Amounts with thousands separators such as `1,250.00` were rejected until `parse_amount` learned to ignore them.
- Yen amounts were shown with cents; the presenter now formats JPY without decimals.

## Run it
```bash
./procpp.sh 47                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_47_gui_architecture       # the interactive demo
ctest --test-dir build -R test_day_47 --output-on-failure
```

## Next step
- Day 48 compares compile-time and run-time typing in one spreadsheet engine.
