// What every test has, as a test sees it: its folder, the user's folders moved into it, its files, a child process and the allocation count.

#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <new>
#include <string>

#include "vkm_test/check.h"
#include "vkm_test/scratch.h"

TEST("a test's folder is its own, under test_scratch and its area") {
    const std::filesystem::path folder = Vkm::Test::scratch();
    CHECK(folder.filename() == "a_test_s_folder_is_its_own_under_test_scratch_and_its_area");
    CHECK(folder.parent_path().filename() == "vkm_test");
    CHECK(folder.parent_path().parent_path().filename() == "test_scratch");
    CHECK(std::filesystem::is_directory(folder / "home"));
}

TEST("the user's folders point at the test's own home") {
    const std::string home = (Vkm::Test::scratch() / "home").string();
    for (const char* folder : {"XDG_CONFIG_HOME", "XDG_STATE_HOME", "APPDATA", "LOCALAPPDATA"}) {
        const char* value = std::getenv(folder);
        REQUIRE(value != nullptr);
        CHECK(std::string(value) == home);
    }
}

TEST("writeText makes the folders a path needs, and readText reads its bytes back") {
    const std::filesystem::path path = Vkm::Test::scratch() / "a" / "b" / "c.bin";
    const std::string bytes = std::string("one\r\ntwo\0three", 14);
    Vkm::Test::writeText(path, bytes);
    CHECK(Vkm::Test::readText(path) == bytes);
    CHECK(Vkm::Test::readText(Vkm::Test::scratch() / "missing.txt").empty());
}

TEST("a child that aborts is seen as stopped, one that returns as returned, any other as otherwise") {
    CHECK(Vkm::Test::inChild([] { std::abort(); }) == Vkm::Test::Ended::Stopped);
    CHECK(Vkm::Test::inChild([] {}) == Vkm::Test::Ended::Returned);
    CHECK(Vkm::Test::inChild([] { throw 1; }) == Vkm::Test::Ended::Otherwise);
    CHECK(Vkm::Test::inChild([] { std::_Exit(5); }) == Vkm::Test::Ended::Otherwise);
}

TEST("the allocation count moves on a new, and stays where nothing allocates") {
    const uint64_t before = Vkm::Test::allocations();
    void* block = ::operator new(16);
    const uint64_t after = Vkm::Test::allocations();
    ::operator delete(block);
    CHECK(after == before + 1);

    int sum = 0;
    for (int i = 0; i < 100; ++i) sum += i;
    const uint64_t still = Vkm::Test::allocations();
    CHECK(still == after);
    CHECK(sum == 4950);
}
