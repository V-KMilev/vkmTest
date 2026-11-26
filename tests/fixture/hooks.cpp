// The fixture suite's hooks, which say when they run, for tests/run.cmake to find in a run's output.

#include <cstdio>

#include "vkm_test/hooks.h"

namespace {

void setUp(const Vkm::Test::Case& test) {
    std::printf("set up %s: %s\n", test.area.c_str(), test.name.c_str());
}

void tearDown(const Vkm::Test::Case& test) {
    std::printf("torn down %s: %s\n", test.area.c_str(), test.name.c_str());
}

const Vkm::Test::Hooks g_hooks(setUp, tearDown);

} // namespace
