# vkmTest

The test harness of vkmEngine and its libraries. A test is a promise in its name and checks in its body; each runs in a process of its own and is a CTest of its own, so a crash, a hang or a process-wide singleton is one test's alone.

```cpp
#include "vkm_test/check.h"

TEST("a line logged parses back as written") {
    const std::optional<Vkm::Log::Line> line = Vkm::Log::parseLine("[2026-01-02 03:04:05.006] [APP] [INFO] hello");
    REQUIRE(line);
    CHECK(line->message == "hello");
    CHECK_NEAR(gain, 0.5f, 0.01f);
}
```

## Conventions

- **A test's full name is `<area>: <what works>`.** The area is the suite's name where its target defines `VKM_TEST_SUITE`, as each library's suite does (`VKM_TEST_SUITE="vkm_log"`); without it, the area is the folder under the last `tests/` in the test's path, so `tests/physics/contacts.cpp` is in `physics`. A test with no area, two tests of one name, or a name holding a `;` stops the binary before anything runs, saying each.
- **`CHECK` goes on and `REQUIRE` stops the test,** so one run shows every check that failed. A failed check prints its expression and, for a comparison, both sides, as `show()` prints a value: an indexed value, such as glm's vectors, quaternions and matrices, part by part; anything with an `operator<<` through it; a container by its first 64 items; anything else as `?`.
- **`CHECK_NEAR(a, b, tolerance)` holds when no part is further apart than the tolerance,** and a NaN part is near nothing. A unit quaternion's q and -q are one rotation, which a part by part compare calls far apart: `CHECK_NEAR(Vkm::Test::nearerSign(rotation, expected), expected, 1e-5f)`.
- **`skip(reason)` ends a test that cannot run on this machine** with exit 77, which CTest shows as skipped, unless a check failed before it.
- **Each test has a folder of its own,** `scratch()`: `test_scratch/<area>/<test>` beside the suite's binary, empty when the test starts, removed after a pass and kept after a failure. While it runs, the user's own folders (`XDG_CONFIG_HOME`, `XDG_STATE_HOME`, `APPDATA`, `LOCALAPPDATA`) point at its `home` folder, so nothing it starts writes the user's files. A test that starts its own binary again for itself sets `VKM_TEST_AGAIN=<name>` for that run, whose folder is `<test>_<name>`, beside its own.
- **The harness opens no log.** A suite that keeps one per test opens it in its `setUp` hook at `logPath()`, `test.log` in the test's folder, and a test reads it back with `logText()` and `countInLog()`.
- **A suite's hooks** run around each of its tests, in the test's process: one `Hooks` object in the suite's own sources, its `setUp` after the test's folder is made and before its body, its `tearDown` after the body however it ended, before the process's statics are destroyed. Each is given the test as a `Case`: its area, name and file.
- `inChild(body)` runs `body` in a child process and says how it ended: `Ended::Returned`, `Stopped` for an abort, or `Otherwise`. `allocations()` counts every `operator new` in the process but a Windows DLL's own, so a test can see a hot path allocate nothing. Each skips the test where it cannot hold: `inChild` on Windows, which has no fork, and `allocations` under a sanitizer.
- `VKM_TEST_LIBRARY_ASSERTS` says whether the library under test has its asserts in: its asserts flag, which the suite passes as `VKM_TEST_ASSERTS_FLAG`, is on, or the test's file has no `NDEBUG`.

## A suite

```cmake
file(GLOB _vkm_log_test_sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_executable(vkm_log_tests ${_vkm_log_test_sources})
target_link_libraries(vkm_log_tests PRIVATE vkm_log vkm_test)
target_compile_definitions(vkm_log_tests PRIVATE VKM_TEST_SUITE="vkm_log")
vkm_discover_tests(vkm_log_tests vkm_log)
```

A suite that logs opens each test's log in its hooks:

```cpp
#include "vkm_log/logger.h"
#include "vkm_test/hooks.h"
#include "vkm_test/scratch.h"

namespace {

void openLog(const Vkm::Test::Case&) { Vkm::Log::init(Vkm::Test::logPath(), "VKM-TESTS", Vkm::Log::Level::Info); }

const Vkm::Test::Hooks g_hooks(openLog);

} // namespace
```

The binary runs one test: `<binary> --list` names them and `<binary> "<area>: <what works>"` runs one, exiting 0 when it passed, 1 when it failed, 77 when it skipped and 2 when it could not run.

| CMake function                                                                     | What                                                                                                                                                                                                                                                                                                                                                                                                 |
|------------------------------------------------------------------------------------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `vkm_discover_tests(<target> <suite> [LABELS <label>...] [WORKING_DIRECTORY <dir>])` | Each test the binary lists is a CTest of its own when CTest runs: named as listed, labeled with its area, `<suite>` and `LABELS`, run in `WORKING_DIRECTORY` or else the binary's folder, skipped on 77, stopped after 300 seconds. Under a multi-config generator it reads the build type `ctest -C` names. A binary that is not built, or cannot list its tests, stands as one failing test, `<suite>: <target> ...`. |
| `vkm_test_consumer(<name> <library> [REQUIRES <target>...])`                       | The CTest `<name>`: the target `<library>`'s folder, added to a project of its own as a consumer adds it, configures with each target it requires added first, and without any one of them stops with an error that says `requires the target '<target>'`.                                                                                                                                           |
| `vkm_test_flags(<name> <library> ASSERTS <bool> [PROFILER <bool>])`                | The CTest `<name>`: the library's sources, `src/<library>/` under its folder, compile with `-UNDEBUG` exactly when `ASSERTS` is on; where they have `NDEBUG` the archive calls no assert; with `PROFILER` off, as by default, it calls nothing of Tracy's. It reads the toolchain's `nm`.                                                                                                             |

The two checks are labeled with their name's prefix, before its colon.

## Requirements

CMake 3.25 or above and a C++17 compiler: GCC or Clang, on Linux or on Windows (MSYS2's UCRT64).

## Use

Add it as a submodule and `add_subdirectory` it; it requires no other target, and once it is added the three functions are defined.

```
git submodule add https://github.com/V-KMilev/vkmTest.git modules/vkmTest
```

The library is `vkm_test`, static, holding the runner's `main`: a suite's binary is its tests and this. Headers are included by the library's folder, `#include "vkm_test/check.h"`; everything is in `Vkm::Test`, but the macros.

| Option                 | Default                                 | What                                                                      |
|------------------------|-----------------------------------------|---------------------------------------------------------------------------|
| `VKM_TEST_BUILD_TESTS` | `ON` built alone, `OFF` as a subproject | Builds `vkm_test_tests`: each test a CTest of its own, labeled `vkm_test` |

## What it holds

| File                                      | What                                                                                                        |
|-------------------------------------------|-------------------------------------------------------------------------------------------------------------|
| `vkm_test/check.h`                        | `TEST`, `CHECK`, `REQUIRE`, `CHECK_NEAR`, `nearerSign`, `show`, `fail`, `Stop`, `VKM_TEST_LIBRARY_ASSERTS` |
| `vkm_test/scratch.h`                      | `scratch`, `writeText`, `readText`, `logPath`, `logText`, `countInLog`, `skip`, `inChild`, `allocations`    |
| `vkm_test/hooks.h`                        | `Hooks` and `Case`, what a suite runs around each test, which the runner's `main` calls                     |
| `cmake/discover.cmake`                    | `vkm_discover_tests`                                                                                        |
| `cmake/consumer.cmake`, `cmake/consumer/` | `vkm_test_consumer`                                                                                         |
| `cmake/flags.cmake`                       | `vkm_test_flags`                                                                                            |

## Tests

`tests/` holds the harness's own suite, built with `VKM_TEST_BUILD_TESTS`: each test is a CTest named `vkm_test: <what works>` and labeled `vkm_test`, so `ctest -L vkm_test` runs them in the tree of a consumer that calls `enable_testing()`. What the harness makes of a test that fails, skips or throws is seen from outside: `tests/fixture/` holds a suite whose tests end each way on purpose, which `tests/run.cmake` runs one at a time and `tests/discovery.cmake` reads back through CTest, and two small libraries, one in a vkm library's shape and one not, for the consumer and flags checks.

## License

vkmTest is source-available under the [vkmEngine License](LICENSE), the engine's own, which holds for each vkm library used alone. [CONTRIBUTING.md](CONTRIBUTING.md) says how a change is shared.
