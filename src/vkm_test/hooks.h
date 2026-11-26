#pragma once

// vkm_test's main runs one test, named on the command line, in this process; ctest starts one process a test (cmake/discover.cmake), so a crash, a hang or a process-wide singleton is one test's alone.
//
//     <suite binary> --list
//     <suite binary> "<area>: <what works>"
//
// A suite says what it does around each of its tests through Hooks, in its own sources.

#include <string>

namespace Vkm::Test {

/// A test, as the suite's hooks see it.
struct Case {
    std::string area;  ///< The prefix of its name: the suite's VKM_TEST_SUITE, or the folder under tests/ that holds its file.
    std::string name;  ///< What works, as TEST names it.
    std::string file;  ///< Its file, as __FILE__ gives it.
};

using Hook = void (*)(const Case& test);

/// Sets, at static initialization, what the suite does around each of its tests, in the test's own process; a binary sets them once.
struct Hooks {
    /**
     * @brief Run @p setUp before each test's body and @p tearDown after it.
     *
     * @param setUp    Once the test's folder is made and the user's folders moved into it, before its body: where a suite opens the test's log, at logPath(). A skip(), REQUIRE or throw in it ends the test as one in its body does. Null for none.
     * @param tearDown After the body, however it ended, before the process's statics are destroyed. Null for none.
     */
    explicit Hooks(Hook setUp, Hook tearDown = nullptr);
};

} // namespace Vkm::Test
