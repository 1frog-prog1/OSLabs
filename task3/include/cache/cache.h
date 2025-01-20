#pragma once

#include <cache/file.h>

#include <filesystem>
#include <string>

#include <sys/file.h>
#include <unistd.h>

namespace Cache {

template <typename Cache>
requires requires(Cache t, std::string s) {
    { s = t.Serialize() };
    { t.Deserialize(s) };
}
class PersistentStorage {
public:
    PersistentStorage(const std::filesystem::path& storagePath)
        : File_(storagePath.lexically_normal())
    {}

    void Store(const Cache& cache) {
        File_.Write(cache.Serialize(), "w");
    }
    
    Cache Load() {
        std::string data;
        File_.Read(&data);
        Cache result;
        result.Deserialize(data);
        return result;
    }
    
    void Lock() {
        File_.Lock();
    }

    void lock() {
        File_.Lock();
    }

    void Unlock() {
        File_.Unlock();
    }

    void unlock() {
        File_.Unlock();
    }

private:
    File::File File_;

};

}
