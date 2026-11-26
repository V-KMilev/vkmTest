// A suite the runner refuses, for tests/run.cmake: two tests of one name, a test with no area, and its hooks set twice.

#include "vkm_test/check.h"
#include "vkm_test/hooks.h"

namespace {

const Vkm::Test::Hooks g_first(nullptr);
const Vkm::Test::Hooks g_second(nullptr);

} // namespace

TEST("a name two tests have") {}

TEST("a name two tests have") {}

#line 1000 "/nowhere/broken.cpp"
TEST("a test in no folder under tests/") {}
