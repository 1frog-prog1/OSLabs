#include <string>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

namespace Serial {

class SerialPort {
public:
    SerialPort(const std::string &portName);
    ~SerialPort();

    void OpenPort();
    void ClosePort();
    std::string ReadData();

    bool IsOpen() const;

private:
#ifdef _WIN32
    bool setupPort();
#endif

    std::string PortName_;

#ifdef _WIN32
    HANDLE Desc_ = NULL;
#else
    int Desc_ = -1;
#endif
    bool Connected_ = false;
};

}
