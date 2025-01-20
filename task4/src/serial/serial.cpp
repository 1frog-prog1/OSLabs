#include <serial/serial.h>

#include <common/public.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <termios.h>
#include <cstring>
#include <unistd.h>
#include <errno.h>
#endif

namespace Serial {

SerialPort::SerialPort(const std::string &portName)
    : PortName_(portName)
{ }

SerialPort::~SerialPort() {
    ClosePort();
}

void SerialPort::OpenPort() {
    if (Connected_) {
        return;
    }

#ifdef _WIN32
    Desc_ = CreateFileA(
        PortName_.c_str(),
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    ASSERT(Desc_ != INVALID_HANDLE_VALUE);

    THROW_ERROR_IF(Desc_ == INVALID_HANDLE_VALUE, "Failed to open serial port " + GetLastError());

    if (!SetupPort()) {
        ClosePort();
        THROW_ERROR("Failed to setup port");
    }

#else
    Desc_ = open(PortName_.c_str(), O_RDWR | O_NOCTTY);
#endif
    if (Desc_ == -1) {
#ifdef _WIN32
        LPSTR messageBuffer = nullptr;
        DWORD errorMessageId = GetLastError();
        size_t size = FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            errorMessageId,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPSTR)&messageBuffer,
            0,
            NULL);

        std::string message(messageBuffer, size);
        LocalFree(messageBuffer);
#else
        std::string message = strerror(errno);
#endif

        ClosePort();
        THROW_ERROR("Failed to open serial port: " + message);
    }

    termios tty;
    if (tcgetattr(Desc_, &tty) != 0) {
#ifdef _WIN32
        LPSTR messageBuffer = nullptr;
        DWORD errorMessageId = GetLastError();
        size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                     NULL, errorMessageId, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

        std::string message(messageBuffer, size);
        LocalFree(messageBuffer);
        std::cerr << "Error from tcgetattr: " << message << std::endl;
#else
        std::string message = strerror(errno);
#endif
        ClosePort();
        THROW_ERROR("tcgetattr ended with exception: " + message);
    }

    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~CRTSCTS;
    tty.c_cflag |= CREAD | CLOCAL;
    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO;
    tty.c_lflag &= ~ECHOE;
    tty.c_lflag &= ~ECHONL;
    tty.c_lflag &= ~ISIG;

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);

    tty.c_oflag &= ~OPOST;

    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 0;

    cfsetospeed(&tty, B9600);
    cfsetispeed(&tty, B9600);

    if (tcsetattr(Desc_, TCSANOW, &tty) != 0) {
#ifdef _WIN32
        LPSTR messageBuffer = nullptr;
        DWORD errorMessageId = GetLastError();
        size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                     NULL, errorMessageId, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

        std::string message(messageBuffer, size);
        LocalFree(messageBuffer);
        std::cerr << "Error from tcsetattr: " << message << std::endl;
#else
        std::string message = strerror(errno);
#endif
        ClosePort();
        THROW_ERROR("tcsetattr ended with exception: " + message);
    }

    Connected_ = true;
}

void SerialPort::ClosePort() {
    if (!Connected_) {
        return;
    }

#ifdef _WIN32
    if (Desc_ != INVALID_HANDLE_VALUE) {
        CloseHandle(Desc_);
        Desc_ = INVALID_HANDLE_VALUE;
    }
#else
    if (Desc_ != -1) {
        close(Desc_);
        Desc_ = -1;
    }
#endif

    Connected_ = false;
}

std::string SerialPort::ReadData() {
    if (!Connected_) {
        return "";
    }

    const int bufferSize = 256;
    char buffer[bufferSize];
    std::string data;

#ifdef _WIN32
    DWORD bytesRead;
    if (!ReadFile(Desc_, buffer, bufferSize - 1, &bytesRead, NULL) || bytesRead == 0) {
        return "";
    }
    buffer[bytesRead] = '\0';
    data = buffer;
#else
    int bytesRead = read(Desc_, buffer, bufferSize - 1);
    if (bytesRead <= 0) {
        return "";
    }
    buffer[bytesRead] = '\0';
    data = buffer;
#endif

    return data;
}

bool SerialPort::IsOpen() const {
    return Connected_;
}

#ifdef _WIN32
bool SerialPort::SetupPort() {
    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams)) {

        LPSTR messageBuffer = nullptr;
        DWORD errorMessageId = GetLastError();
        size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                     NULL, errorMessageId, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

        std::string message(messageBuffer, size);
        LocalFree(messageBuffer);
        std::cerr << "Error getting port state: " << message << std::endl;

        return false;
    }

    dcbSerialParams.BaudRate = CBR_9600;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

    if (!SetCommState(hSerial, &dcbSerialParams)) {

        LPSTR messageBuffer = nullptr;
        DWORD errorMessageId = GetLastError();
        size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                     NULL, errorMessageId, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

        std::string message(messageBuffer, size);
        LocalFree(messageBuffer);
        std::cerr << "Error setting port state: " << message << std::endl;
        return false;
    }

    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    if (!SetCommTimeouts(hSerial, &timeouts)) {

        LPSTR messageBuffer = nullptr;
        DWORD errorMessageId = GetLastError();
        size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                     NULL, errorMessageId, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

        std::string message(messageBuffer, size);
        LocalFree(messageBuffer);
        std::cerr << "Error setting timeouts: " << message << std::endl;
        return false;
    }

    return true;
}
#endif

}
