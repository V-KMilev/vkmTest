#include "vkm_test/scratch.h"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <system_error>

#if !defined(_WIN32)
    #include <csignal>

    #include <sys/resource.h>
    #include <sys/wait.h>
    #include <unistd.h>
#endif

#include "vkm_test/check.h"

namespace Vkm::Test {

void writeText(const std::filesystem::path& path, const std::string& text) {
    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);
    std::ofstream(path, std::ios::binary) << text;
}

std::string readText(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    std::stringstream text;
    text << file.rdbuf();
    return text.str();
}

std::filesystem::path logPath() { return scratch() / "test.log"; }

std::string logText() { return readText(logPath()); }

int countInLog(const std::string& text) {
    if (text.empty()) {
        fail(__FILE__, __LINE__, "countInLog was given no text to count");
        return 0;
    }
    const std::string log = logText();
    int count = 0;
    for (size_t at = log.find(text); at != std::string::npos; at = log.find(text, at + text.size())) ++count;
    return count;
}

Ended inChild(const std::function<void()>& body) {
#if defined(_WIN32)
    (void)body;
    skip("a child process here needs fork");
#else
    std::fflush(nullptr);
    const pid_t child = fork();
    if (child == 0) {
        // An abort is the expected end: no core file for it.
        const rlimit none{0, 0};
        setrlimit(RLIMIT_CORE, &none);
        try {
            body();
        } catch (...) {
            _exit(3);
        }
        std::fflush(nullptr);
        _exit(0);
    }
    int status = 0;
    if (child < 0 || waitpid(child, &status, 0) != child) return Ended::Otherwise;
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) return Ended::Returned;
    if (WIFSIGNALED(status) && WTERMSIG(status) == SIGABRT) return Ended::Stopped;
    return Ended::Otherwise;
#endif
}

} // namespace Vkm::Test
