#include <common/public.h>
#include <cache/file.h>

#include <cache/cache.h>
#include <subprocess/subprocess.h>

#include <iostream>
#include <filesystem>
#include <sstream>
#include <ctime>
#include <chrono>
#include <thread>

namespace {

struct Data {
    int subprocessCount = 0;

    uint64_t counter = 0;

    std::string Serialize() const {
        return std::string() + std::to_string(counter) + " " + std::to_string(subprocessCount);
    }

    void Deserialize(const std::string& data) {
        std::istringstream ss(data);
        ss >> counter >> subprocessCount;
    }
};

std::string CurrentTimeString() {
    std::time_t now = std::time(nullptr);
    std::string str = std::ctime(&now);
    str.pop_back(); // std::ctime оканчивает строку переносом строки :(
    return str;
}

std::string GetPid()
{
#ifdef WIN32
    return std::to_string(GetCurrentProcessId());
#else
    return std::to_string(getpid());
#endif
}

std::filesystem::path CacheDir() {
    return std::filesystem::temp_directory_path() / "counter_data" / "cache.bin";
}

std::filesystem::path LeadingProcessLockDir() {
    return std::filesystem::temp_directory_path() / "counter_data" / "leading_process.bin";
}

void Log(const std::string& message) {
    File::File logFile("logs/log.txt");
    auto guard = std::lock_guard(logFile);
    logFile.Write(message + '\n', "a");
}

void Increment(std::shared_ptr<File::LockingFile> leadLock) {
    Cache::PersistentStorage<Data> cache(CacheDir());

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        auto guard = std::lock_guard(cache);
        if (leadLock->TryLock()) {
            auto data = cache.Load();
            data.counter++;
            Log("Counter: " + std::to_string(data.counter));
            cache.Store(data);
        }
    }
}

void DatePid() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        Log("Time: " + CurrentTimeString() + ", Pid: " + GetPid());
    }
}

void AddTenProcess() {
    Cache::PersistentStorage<Data> cache(CacheDir());
    
    // Нет смысла обозначать, поскольку лочка не даст другому потоку проверить счетчик
    auto guard = std::lock_guard(cache);
    Log("Time: " + CurrentTimeString() + ", Pid: " + GetPid());
    auto data = cache.Load();
    data.counter += 10;
    cache.Store(data);
}

void MultDevideDoubleProcess() {
    Cache::PersistentStorage<Data> cache(CacheDir());
    {
        auto guard = std::lock_guard(cache);
        Log("Time: " + CurrentTimeString() + ", Pid: " + GetPid());

        auto data = cache.Load();
        data.subprocessCount++;
        data.counter *= 2;
        cache.Store(data);
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));
    {
        auto guard = std::lock_guard(cache);

        auto data = cache.Load();
        data.counter /= 2;
        data.subprocessCount--;
        cache.Store(data);
    }
}


void CreateCopies(std::shared_ptr<File::LockingFile> leadLock, const std::string& cmd) {
    Cache::PersistentStorage<Data> cache(CacheDir());

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        if (leadLock->TryLock()) {
            auto guard = std::lock_guard(cache);
            auto data = cache.Load();
            if (data.subprocessCount > 0) {
                continue;
            }

            Subprocess::subprocess(cmd + " ten");
            Subprocess::subprocess(cmd + " double");
        }
    }
}

} // namespace

int main(int argc, char *argv[])
{
    ASSERT(argc <= 2);
    std::string mode = argc == 2 ? argv[1] : "";

    Log("Time: " + CurrentTimeString() + ", Pid: " + GetPid());
    if (mode == "ten") {
        AddTenProcess();
        return 0;
    }

    if (mode == "double") {
        MultDevideDoubleProcess();
        return 0;
    }

    auto leadLock = std::make_shared<File::LockingFile>(LeadingProcessLockDir());
    std::thread(&Increment, leadLock).detach();
    std::thread(&DatePid).detach();
    std::thread(&CreateCopies, leadLock, std::string(argv[0])).detach();

    while (true) {
        int value;
        std::cin >> value;

        Cache::PersistentStorage<Data> cache(CacheDir());
        auto guard = std::lock_guard(cache);
        auto data = cache.Load();
        data.counter = value;
        cache.Store(data);
    }
}
