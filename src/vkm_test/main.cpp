#include "vkm_test/hooks.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <filesystem>
#include <map>
#include <string>
#include <system_error>
#include <vector>

#include "vkm_test/check.h"
#include "vkm_test/scratch.h"

// Last, as its macros reach only what follows.
#if defined(_WIN32)
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #ifndef NOGDI
        #define NOGDI
    #endif
    #include <windows.h>
#endif

namespace Vkm::Test {

namespace {

struct Entry {
    Case test;
    Body body = nullptr;
};

struct SuiteHooks {
    Hook setUp    = nullptr;
    Hook tearDown = nullptr;
};

std::vector<Entry>& entries() {
    static std::vector<Entry> s_entries;
    return s_entries;
}

std::vector<SuiteHooks>& hookSets() {
    static std::vector<SuiteHooks> s_hookSets;
    return s_hookSets;
}

int g_failures = 0;

std::filesystem::path g_scratch;

/// The folder under the last tests/ in @p file's path, either slash a separator; empty when the file is in no folder under one.
std::string areaOf(const std::string& file) {
    std::vector<std::string> folders;
    std::string part;
    for (const char c : file) {
        if (c != '/' && c != '\\') {
            part += c;
            continue;
        }
        if (!part.empty()) folders.push_back(part);
        part.clear();
    }
    for (size_t after = folders.size(); after > 1; --after) {
        if (folders[after - 2] == "tests") return folders[after - 1];
    }
    return {};
}

/// @p name as a folder name: what a file system takes, nothing else.
std::string folderFor(const std::string& name) {
    std::string out;
    for (const char c : name) {
        const bool plain = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
        if (plain) out += c;
        else if (!out.empty() && out.back() != '_') out += '_';
    }
    return out.substr(0, 80);
}

/// The folder the suite's binary is in, asked of the system as it runs: a path built in would name the building machine's, which a binary built for another system runs without.
std::filesystem::path executableDir() {
#if defined(_WIN32)
    wchar_t path[MAX_PATH];
    const DWORD length = GetModuleFileNameW(nullptr, path, MAX_PATH);
    if (length == 0 || length >= MAX_PATH) return {};  // failed or cut short
    return std::filesystem::path(path, path + length).parent_path();
#else
    std::error_code ec;
    return std::filesystem::read_symlink("/proc/self/exe", ec).parent_path();
#endif
}

void setEnvironment(const char* key, const std::filesystem::path& value) {
#if defined(_WIN32)
    _putenv_s(key, value.string().c_str());
#else
    setenv(key, value.c_str(), 1);
#endif
}

/// Points the user's own folders at @p home. vkm's pinned tools stay where the user's LOCALAPPDATA keeps them, as VKM_TOOLS_DIR, so a test that runs vkm finds them and never fetches them again into its folder.
void moveUserFolders(const std::filesystem::path& home) {
    const char* local = std::getenv("LOCALAPPDATA");
    if (local && !std::getenv("VKM_TOOLS_DIR")) {
        setEnvironment("VKM_TOOLS_DIR", std::filesystem::path(local) / "vkm" / "tools");
    }
    for (const char* folder : {"XDG_CONFIG_HOME", "XDG_STATE_HOME", "APPDATA", "LOCALAPPDATA"}) {
        setEnvironment(folder, home);
    }
}

/// Run @p entry, named @p full, in a folder of its own between the suite's @p hooks: 0 passed, 1 failed, 77 skipped, 2 could not run.
int run(const std::string& full, const Entry& entry, const SuiteHooks& hooks) {
    const std::filesystem::path home = executableDir();
    if (home.empty()) {
        std::printf("The suite's own folder cannot be found: '%s' has nowhere to run.\n", full.c_str());
        return 2;
    }
    g_scratch = home / "test_scratch" / entry.test.area / folderFor(entry.test.name);
    // A test that starts its own binary again for itself names that run in VKM_TEST_AGAIN, whose folder sits beside its own.
    if (const char* again = std::getenv("VKM_TEST_AGAIN")) g_scratch += std::string("_") + again;
    std::error_code ec;
    std::filesystem::remove_all(g_scratch, ec);
    if (!ec) std::filesystem::create_directories(g_scratch / "home", ec);
    if (ec) {
        std::printf(
            "'%s' has no empty folder at %s: %s.\n",
            full.c_str(),
            g_scratch.string().c_str(),
            ec.message().c_str()
        );
        return 2;
    }
    moveUserFolders(g_scratch / "home");

    std::printf("%s\n", full.c_str());
    bool skipped = false;
    try {
        if (hooks.setUp) hooks.setUp(entry.test);
        entry.body();
    } catch (const Stop&) {
    } catch (const Skip& why) {
        std::printf("SKIPPED: %s\n", why.reason.c_str());
        skipped = true;
    } catch (const std::exception& e) {
        fail("", 0, std::string("threw ") + e.what());
    }
    if (hooks.tearDown) hooks.tearDown(entry.test);

    // A check that failed before the skip still fails the test.
    if (skipped && g_failures == 0) return 77;
    std::printf(g_failures == 0 ? "ok\n" : "FAILED\n");
    return g_failures == 0 ? 0 : 1;
}

} // namespace

Registration::Registration(const char* suite, const char* file, const char* name, Body body) {
    entries().push_back({{suite ? suite : areaOf(file), name, file}, body});
}

Hooks::Hooks(Hook setUp, Hook tearDown) { hookSets().push_back({setUp, tearDown}); }

void fail(const char* file, int line, const std::string& what) {
    ++g_failures;
    const std::string name  = std::filesystem::path(file).filename().string();
    const std::string where = file[0] ? name + ":" + std::to_string(line) : std::string("test");
    std::printf("  %s: %s\n", where.c_str(), what.c_str());
}

const std::filesystem::path& scratch() { return g_scratch; }

} // namespace Vkm::Test

int main(int argc, char** argv) {
    using namespace Vkm::Test;

    // Unbuffered, as ctest reads it through a pipe: a crash keeps what was printed before it.
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    // What the suite registered, checked before anything runs, each problem said.
    bool sound = true;
    std::map<std::string, const Entry*> byName;
    for (const Entry& entry : entries()) {
        const Case& test = entry.test;
        if (test.area.empty()) {
            std::printf(
                "'%s' in %s has no area: its suite defines no VKM_TEST_SUITE, and its file is in no folder "
                "under tests/.\n",
                test.name.c_str(),
                test.file.c_str()
            );
            sound = false;
            continue;
        }
        const std::string full = test.area + ": " + test.name;
        // ctest's lists split on ';'.
        if (full.find(';') != std::string::npos) {
            std::printf("A test's name holds a ';': '%s'.\n", full.c_str());
            sound = false;
        }
        if (!byName.emplace(full, &entry).second) {
            std::printf("Two tests are called '%s'.\n", full.c_str());
            sound = false;
        }
    }
    if (hookSets().size() > 1) {
        std::printf("The suite sets its hooks %zu times; a binary sets them once.\n", hookSets().size());
        sound = false;
    }
    if (!sound) return 2;

    if (argc == 2 && std::strcmp(argv[1], "--list") == 0) {
        for (const auto& [full, entry] : byName) std::printf("%s\n", full.c_str());
        return 0;
    }
    if (argc != 2) {
        std::printf("Usage: %s --list | \"<area>: <what works>\"\n", argv[0]);
        return 2;
    }
    const auto found = byName.find(argv[1]);
    if (found == byName.end()) {
        std::printf("No test called '%s'; --list names them.\n", argv[1]);
        return 2;
    }

    const SuiteHooks hooks = hookSets().empty() ? SuiteHooks{} : hookSets().front();
    const int status = run(found->first, *found->second, hooks);
    if (status == 0) {
        std::error_code ec;
        std::filesystem::remove_all(g_scratch, ec);
    }
    return status;
}
