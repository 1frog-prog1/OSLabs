#include <cache/file.h>
#include <common/public.h>

#include <sys/file.h>
#include <unistd.h>

namespace File {

namespace {

struct DescHolder {
    DescHolder(int value)
        : Desc_(value)
    {}

    ~DescHolder() {
        if (Desc_) {
            close(Desc_);
            Desc_ = 0;
        }
    }

    int Release() {
        int temp = Desc_;
        Desc_ = 0;
        return temp;
    }

    int Desc_;
};

struct FileDescHolder {
    FileDescHolder(FILE* value)
        : Desc_(value)
    {}

    ~FileDescHolder() {
        if (Desc_) {
            fclose(Desc_);
            Desc_ = nullptr;
        }
    }

    FILE* Release() {
        FILE* temp = Desc_;
        Desc_ = nullptr;
        return temp;
    }

    FILE* Desc_;
};

#if defined(_WIN32) || defined(_WIN64)
struct HandleHolder {
    HandleHolder(HANDLE value)
        : Desc_(value)
    {}

    ~HandleHolder() {
        if (Desc_) {
            CloseHandle(Desc_);
            Desc_ = NULL;
        }
    }

    HANDLE Release() {
        HANDLE temp = Desc_;
        Desc_ = NULL;
        return temp;
    }

    HANDLE Desc_;
};
#endif

}

LockingFile::LockingFile() = default;

LockingFile::LockingFile(const std::filesystem::path& filepath)
    : FilePath_(filepath)
{}

LockingFile::~LockingFile() {
    auto guard = std::lock_guard(Mutex_);
    if (LockFileDesc_) {
        Unlock();
    }
}

LockingFile& LockingFile::operator=(LockingFile&& other) {
    Move(&other);
    return *this;
}

void LockingFile::Move(LockingFile* other) {
    if (other != this) {
        Unlock();
        LockFileDesc_ = other->LockFileDesc_;
        FilePath_ = std::move(other->FilePath_);

        other->LockFileDesc_ = 0;
    }
}

bool LockingFile::Valid() const {
    return !FilePath_.empty();
}

void LockingFile::Lock() {
    auto guard = std::lock_guard(Mutex_);

    if (IsLocked()) {
        return;
    }

#if defined(_WIN32) || defined(_WIN64)
    HandleHolder hFile = CreateFileA(FilePath_.c_str(), GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    ASSERT(hFile.Desc_ != INVALID_HANDLE_VALUE);

    OVERLAPPED ov = {0};
    ASSERT(LockFileEx(hFile.Desc_, LOCKFILE_EXCLUSIVE_LOCK, 0, MAXDWORD, MAXDWORD, &ov));
    LockFileDesc_ = HandleHolder.Release();
#else
    std::filesystem::create_directory(FilePath_.parent_path());
    DescHolder TempFileDesc_ = open(FilePath_.c_str(), O_CREAT | O_RDONLY, 0666);
    ASSERT(TempFileDesc_.Desc_ >= 0);
    ASSERT(!flock(TempFileDesc_.Desc_, LOCK_EX));
    LockFileDesc_ = TempFileDesc_.Release();
#endif
}

void LockingFile::lock() {
    Lock();
}

bool LockingFile::TryLock() {
    auto guard = std::lock_guard(Mutex_);

    if (IsLocked()) {
        return true;
    }

    std::filesystem::create_directory(FilePath_.parent_path());

#if defined(_WIN32) || defined(_WIN64)
    HandleHolder hFile = CreateFileA(FilePath_.c_str(), GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    ASSERT(hFile.Desc_ != INVALID_HANDLE_VALUE);

    OVERLAPPED ov = {0};
    if (LockFileEx(hFile.Desc_, LOCKFILE_EXCLUSIVE_LOCK | LOCKFILE_FAIL_IMMEDIATELY, 0, MAXDWORD, MAXDWORD, &ov))) {
        return false;
    }
    LockFileDesc_ = HandleHolder.Release();
    return true;
#else
    DescHolder TempFileDesc_ = open(FilePath_.c_str(), O_CREAT | O_RDONLY, 0666);
    ASSERT(TempFileDesc_.Desc_ >= 0);

    if (flock(TempFileDesc_.Desc_, LOCK_EX | LOCK_NB)) {
        return false;
    }

    LockFileDesc_ = TempFileDesc_.Release();
    return true;
#endif
}

void LockingFile::Unlock() {
    auto guard = std::lock_guard(Mutex_);

    if (!IsLocked()) {
        return;
    }

#if defined(_WIN32) || defined(_WIN64)
    OVERLAPPED ov = {0};
    ASSERT(UnlockFileEx(LockFileDesc_, 0, MAXDWORD, MAXDWORD, &ov));
    ASSERT(CloseHandle(LockFileDesc_));
#else
    ASSERT(!flock(LockFileDesc_, LOCK_UN));
    ASSERT(close(LockFileDesc_) >= 0);
    LockFileDesc_ = 0;
#endif
}

void LockingFile::unlock() {
    return Unlock();
}

bool LockingFile::IsLocked() const {
    return LockFileDesc_;
}

File& File::operator=(File&& other) {
    Move(&other);
    return *this;
}

void File::Write(const std::string& data, const char* mode) {
    auto guard = std::lock_guard(*this);

    FileDescHolder WriteFileDesc_ = fopen(FilePath_.c_str(), mode);
    ASSERT(WriteFileDesc_.Desc_);
    fprintf(WriteFileDesc_.Desc_, "%s", data.data());
}

void File::Read(std::string* output) {
    auto guard = std::lock_guard(*this);

    FILE* ReadFileDesc_ = fopen(FilePath_.c_str(), "r");
    ASSERT(ReadFileDesc_);

    fseek(ReadFileDesc_, 0, SEEK_END); 
    long long fileSize = ftell(ReadFileDesc_);
    fseek(ReadFileDesc_, 0, SEEK_SET); 

    output->resize(fileSize);
    fread(output->data(), fileSize, 1, ReadFileDesc_);
}

}
