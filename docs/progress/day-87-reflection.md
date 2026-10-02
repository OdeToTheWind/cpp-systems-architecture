# Day 87 – Plugin-style Architecture Reflection

**Date:** 2026-06-08 · **Level:** Capstone · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_87_plugins/lesson.hpp`](../../src/day_87_plugins/lesson.hpp) · **Tests:** [`tests/test_day_87.cpp`](../../tests/test_day_87.cpp) (7 tests)

## Scenario
A *photo-filter tool* whose filters come from plugins. The host knows only the plugin API (plugin_api.hpp): plugins register themselves with a registry, the host creates configured instances through their factories by name, and plugins written for an incompatible API version are refused with a reason instead of crashing at run time.

## Syllabus deliverables
> Plugin interfaces, self-registration, factories, versioned compatibility checks

| Deliverable | Implemented in |
|---|---|
| ✅ the interface every plugin implements | `Filter` |
| ✅ plugin metadata with a factory and required API | `PluginInfo` |
| ✅ version checks when a plugin registers | `PluginRegistry::add` |
| ✅ self-registration from a plugin header | `register_plugin` |
| ✅ building a pipeline from plugin names and options | `build_pipeline` |

## Key learnings
- A plugin system starts with a small, stable interface; the host depends only on that, never on concrete plugins.
- Plugins describe themselves (name, description, required API) and provide a factory, so the host creates configured instances by name.
- A function-local static registry avoids the static initialisation order fiasco when plugins register during start-up.
- Checking the API version at registration (same major, minor not newer than the host) turns incompatibility into a clear message instead of a crash.

## Pitfalls I hit (and how I fixed them)
- Self-registering plugins defined only in .cpp files of a static library were silently dropped by the linker, because nothing referenced them; registration now lives in headers via inline variables.
- A second plugin with the same name replaced the first; duplicates are now refused and reported.
- Option values like level=300 crashed inside a filter; options are validated in the factory.

## Run it
```bash
./procpp.sh 87                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_87_plugins       # the interactive demo
ctest --test-dir build -R test_day_87 --output-on-failure
```

## Next step
- Day 88 generates reports with exact money and safe escaping.
