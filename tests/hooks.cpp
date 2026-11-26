// The suite's hooks, as a suite that keeps a log sets them: setUp opens the test's log at logPath() by writing its first line, which the tests read back.

#include <filesystem>
#include <fstream>
#include <string>

#include "vkm_test/check.h"
#include "vkm_test/hooks.h"
#include "vkm_test/scratch.h"

namespace {

void setUp(const Vkm::Test::Case& test) {
    const std::string file = std::filesystem::path(test.file).filename().string();
    Vkm::Test::writeText(
        Vkm::Test::logPath(),
        "set up " + test.area + ": " + test.name + " from " + file + "\n"
    );
}

const Vkm::Test::Hooks g_hooks(setUp);

} // namespace

TEST("setUp runs before the body, given the test, and the log it opens is at logPath()") {
    CHECK(Vkm::Test::logPath() == Vkm::Test::scratch() / "test.log");
    const std::string opened =
        "set up vkm_test: setUp runs before the body, given the test, and the log it opens is at logPath() "
        "from hooks.cpp\n";
    CHECK(Vkm::Test::logText() == opened);
}

TEST("countInLog counts each time a text is in the test's log") {
    std::ofstream(Vkm::Test::logPath(), std::ios::app) << "a warning\nanother warning, a warning\n";
    CHECK(Vkm::Test::countInLog("warning") == 3);
    CHECK(Vkm::Test::countInLog("a warning") == 2);
    CHECK(Vkm::Test::countInLog("an error") == 0);
}
