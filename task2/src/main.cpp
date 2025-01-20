#include <subprocess/subprocess.h>

#include <iostream>
#include <stdexcept>
#include <unordered_set>

#define TOSTRING(token) #token
#define ASSERT(condition) if (!(condition)) { throw std::runtime_error(std::string() + "Condition is false (" TOSTRING(condition) ") in " + __PRETTY_FUNCTION__ + " -- " + __FILE__ + ":" + std::to_string(__LINE__)); }

bool checkAns(std::string str) {
    static std::unordered_set<std::string_view> Positive = {
        "yes", "y", "1", "on", "true"
    };
    static std::unordered_set<std::string_view> Negative = {
        "no", "n", "0", "off", "false"
    };

    for (auto& c : str) {
        c = std::tolower(c);
    }

    if (Positive.contains(str)) {
        return true;
    }
    if (Negative.contains(str)) {
        return false;
    }
    throw std::runtime_error("Failed to parse argument, print yes or no");
}

int main(int argc, char *argv[])
{
    ASSERT(argc == 3);
    bool mustWait = checkAns(argv[2]);

    int pid = Subprocess::subprocess(argv[1]);
    int exitCode = 0, status = 0;

    if (mustWait) {
        auto [e, s] = Subprocess::waitProcess(pid);

        exitCode = e;
        status = s;
    }

    std::cout << "Pid: " << pid << ", Code: " << exitCode << ", Status: " << status << std::endl;
}
