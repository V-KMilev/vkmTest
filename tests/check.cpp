// check.h's checks where they hold, and how a failed one prints a value; what a failed check does to its test is run.cmake's to see (tests/CMakeLists.txt).

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

#include "vkm_test/check.h"

namespace {

/// A value of two parts, indexed as a vector is.
struct Pair {
    float parts[2] = {0.0f, 0.0f};

    static constexpr int length() { return 2; }
    float operator[](int i) const { return parts[i]; }
};

/// A quaternion as glm's is to the harness: four parts, indexed, and a negation.
struct Quat {
    float parts[4] = {1.0f, 0.0f, 0.0f, 0.0f};

    static constexpr int length() { return 4; }
    float operator[](int i) const { return parts[i]; }
    Quat operator-() const { return {{-parts[0], -parts[1], -parts[2], -parts[3]}}; }
};

enum class Kind {
    Plain = 3,
};

struct Opaque {};

} // namespace

TEST("show prints a bool as a word, an enum and a byte as numbers, and a string in quotes") {
    const char* none = nullptr;
    CHECK(Vkm::Test::show(true) == "true");
    CHECK(Vkm::Test::show(Kind::Plain) == "3");
    CHECK(Vkm::Test::show(uint8_t{65}) == "65");
    CHECK(Vkm::Test::show(std::string("text")) == "\"text\"");
    CHECK(Vkm::Test::show("text") == "text");
    CHECK(Vkm::Test::show(none) == "null");
    CHECK(Vkm::Test::show(1.5f) == "1.5");
}

TEST("show prints an indexed value part by part and a container's first 64 items") {
    CHECK(Vkm::Test::show(Pair{{1.0f, 2.5f}}) == "(1, 2.5)");
    CHECK(Vkm::Test::show(std::vector<int>{1, 2, 3}) == "{1, 2, 3}");

    const std::vector<int> many(70, 7);
    const std::string shown = Vkm::Test::show(many);
    CHECK(shown.substr(shown.size() - 10) == "7, 7, ...}");
    CHECK(static_cast<int>(std::count(shown.begin(), shown.end(), '7')) == 64);
}

TEST("show prints a value it cannot as a question mark") { CHECK(Vkm::Test::show(Opaque{}) == "?"); }

TEST("CHECK_NEAR holds within its tolerance, on a scalar and on every part of an indexed value") {
    CHECK_NEAR(1.0f, 1.0005f, 1e-3f);
    CHECK_NEAR(1, 1.0005, 1e-3f);
    CHECK_NEAR((Pair{{1.0f, 2.0f}}), (Pair{{1.0005f, 1.9995f}}), 1e-3f);
}

TEST("a REQUIRE that holds goes on") {
    bool reached = false;
    REQUIRE(1 + 1 == 2);
    reached = true;
    CHECK(reached);
}

TEST("nearerSign turns a quaternion's negation back, so q and -q compare as one rotation") {
    const Quat turned = {{0.5f, 0.5f, 0.5f, 0.5f}};
    const Quat other;
    CHECK_NEAR(Vkm::Test::nearerSign(-turned, turned), turned, 0.0f);
    CHECK_NEAR(Vkm::Test::nearerSign(turned, turned), turned, 0.0f);
    CHECK(Vkm::Test::nearerSign(other, turned)[0] == 1.0f);
}

TEST("VKM_TEST_LIBRARY_ASSERTS is on under the library's asserts flag, whatever the build type") {
    CHECK(VKM_TEST_LIBRARY_ASSERTS);
}
