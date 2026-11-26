#pragma once

// What every test has: a directory of its own, the log a suite opens there, a way to say it cannot run here, a child process to stop in, and a count of allocations.

#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>

namespace Vkm::Test {

/// Thrown by skip(): the test cannot run on this machine, which ctest shows as skipped (exit 77) unless a check failed before it.
struct Skip {
    std::string reason;
};

/// Stop the test as skipped, saying why: a missing device, never a failing check; one that failed before it still fails the test.
[[noreturn]] inline void skip(const std::string& reason) { throw Skip{reason}; }

/**
 * @brief This test's own directory, test_scratch/<area>/<test> beside the suite's binary, empty when it starts.
 *
 * Removed after a pass, kept after a failure for what it wrote. The user's own folders, $XDG_CONFIG_HOME, $XDG_STATE_HOME, %APPDATA% and %LOCALAPPDATA%, point at its `home` folder while the test runs, so nothing it starts writes the user's files.
 */
const std::filesystem::path& scratch();

/**
 * @brief Write @p text to @p path, making its folders.
 *
 * @param path Where.
 * @param text What, as bytes.
 */
void writeText(const std::filesystem::path& path, const std::string& text);

/// The bytes of @p path; empty when it will not open.
std::string readText(const std::filesystem::path& path);

/// Where a suite's setUp hook opens the test's log (hooks.h): test.log in scratch(), which logText() and countInLog() read.
std::filesystem::path logPath();

/// Everything the test's log holds so far; empty where the suite opens none.
std::string logText();

/// How many times @p text appears in the test's log so far.
int countInLog(const std::string& text);

/// How a child process ended.
enum class Ended {
    Returned,   ///< Its body returned.
    Stopped,    ///< It aborted, as a failed assert stops a process.
    Otherwise,  ///< Any other way: a throw, another signal, an exit of its own.
};

/**
 * @brief Run @p body in a child process of its own and wait for it.
 *
 * A check inside @p body counts in the child alone; the test reads what it left behind (its exit, a file it wrote). Skips the test where there is no fork.
 *
 * @param body What the child runs.
 * @return How the child ended.
 */
Ended inChild(const std::function<void()>& body);

/// How many times the process has called operator new so far, in any form, on any thread: a hot path that allocates nothing leaves it where it was. It skips the test on Windows, where a DLL's own calls are not counted, and under a sanitizer, which brings its own operator new.
uint64_t allocations();

} // namespace Vkm::Test
