// The fixture suite: tests that fail, skip and throw on purpose, each run alone by tests/run.cmake, which checks what came of it.

#include <cstdio>
#include <filesystem>
#include <limits>
#include <stdexcept>

#include "vkm_test/check.h"
#include "vkm_test/scratch.h"

namespace {

/// A value of two parts, indexed as a vector is.
struct Pair {
    float parts[2] = {0.0f, 0.0f};

    static constexpr int length() { return 2; }
    float operator[](int i) const { return parts[i]; }
};

} // namespace

TEST("a CHECK that fails, then another") {
    const int one = 1;
    const int two = 2;
    CHECK(one == two);
    CHECK(two < one);
}

TEST("a CHECK_NEAR on a value with a NaN part") {
    const float nan     = std::numeric_limits<float>::quiet_NaN();
    const Pair  withNaN = {{0.0f, nan}};
    const Pair  zero;
    CHECK_NEAR(withNaN, zero, 1.0f);
    CHECK_NEAR(nan, 0.0f, 1.0f);
}

TEST("a REQUIRE that fails") {
    const int one = 1;
    const int two = 2;
    REQUIRE(one == two);
    std::printf("after the REQUIRE\n");
}

TEST("a test that skips") { Vkm::Test::skip("no device here"); }

TEST("a test that fails a check, then skips") {
    CHECK(1 + 1 == 3);
    Vkm::Test::skip("no device here");
}

TEST("a test that throws") { throw std::out_of_range("past the end"); }

TEST("a test that passes") { std::printf("in the body\n"); }

TEST("a test that passes, writing in its folder") {
    Vkm::Test::writeText(Vkm::Test::scratch() / "kept.txt", "written");
    CHECK(Vkm::Test::readText(Vkm::Test::scratch() / "kept.txt") == "written");
}

TEST("a test that fails, writing in its folder") {
    Vkm::Test::writeText(Vkm::Test::scratch() / "kept.txt", "written");
    CHECK(false);
}

TEST("a test run again, writing in its folder") {
    Vkm::Test::writeText(Vkm::Test::scratch() / "kept.txt", "written");
    CHECK(false);
}
