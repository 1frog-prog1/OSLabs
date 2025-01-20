#include <subprocess/subprocess.h>

#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <spawn.h>
#include <string.h>
#include <wait.h>
#endif

namespace Subprocess {

int subprocess(std::string command) {
#ifdef _WIN32
    STARTUPINFO si{};
    PROCESS_INFORMATION pi;
    int success = CreateProcess(
        command.data(), // The path
        nullptr, // Command line
        nullptr, // Process handle not inheritable
        nullptr, // Thread handle not inheritable
        FALSE, // Set handle inheritance to FALSE
        0, // No creation flags
        nullptr, // Use parent's environment block
        nullptr, // Use parent's starting directory
        &si, // Pointer to STARTUPINFO structure
        &pi); // Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
    

    if (success == 0) {
        status = GetLastError();
    }

    return pi.dwProcessId;
#else
    pid_t pid;

    char* data = command.data();
    std::vector<char*> args;
    while (auto token = strtok_r(data, " ", &data)) {
        args.push_back(token);
    }

    int status = posix_spawnp(
        &pid, // Pid output
        args[0], // File to execute
        nullptr, // No actions between fork and exec
        nullptr, // Standart attributes for process
        args.data(), // Arguments for process
        nullptr); // Use parent's environment

    return pid;
#endif
}

std::pair<int, int> waitProcess(int pid)
{
#ifdef _WIN32
    HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    int status = WaitForSingleObject(handle, INFINITE);
    unsign long exitStatus;
    GetExitCodeProcess(handle, &exitStatus);
    CloseHandle(handle);
    return { status, exitStatus };
#else
    int status;
    waitpid(pid, &status, 0);
    return { status, WEXITSTATUS(status) };
#endif
}

}
