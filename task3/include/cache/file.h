#pragma once

#include <filesystem>
#include <mutex>

namespace File {

class LockingFile {
public:
    LockingFile();

    LockingFile(const std::filesystem::path& filepath);

    LockingFile(LockingFile&& file);

    ~LockingFile();

    LockingFile& operator=(LockingFile&& other);

    bool Valid() const;

    bool IsLocked() const;

    void Lock();
    void Unlock();
    bool TryLock();

    // Для совместимости с STL
    void lock(); 
    void unlock();

protected:

    void Move(LockingFile* other);

    std::filesystem::path FilePath_;

#if defined(_WIN32) || defined(_WIN64)
    HANDLE LockFileDesc_ = NULL;
#else
    int LockFileDesc_ = 0;
#endif

    std::mutex Mutex_;

};

class File
    : public LockingFile
{
public:
    using LockingFile::LockingFile;

    File& operator=(File&& other);

    void Write(const std::string& data, const char* mode);

    void Read(std::string* output);

};

}
