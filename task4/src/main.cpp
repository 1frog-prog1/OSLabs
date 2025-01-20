#include <common/public.h>
#include <temperature_storage.h>
#include <serial/serial.h>

#include <thread>
#include <chrono>
#include <random>
#include <cmath>

namespace {

#define DAY 3600 * 24
double SimulateTemperature(time_t time) {
    auto tm = std::localtime(&time);
    
    long long seconds = time / DAY;
    long long days = tm->tm_yday;
    
    return - std::cos(365.0 / days) * 25 - std::sin(double(DAY) / seconds) * 10;
}

}

int main(int argc, char** argv) {
    ASSERT(argc >= 3);
    TemperatureStorage::TemperatureStorage storage(argv[1]);
    if (std::string(argv[2]) == "simulate") {
        while (true) {
            auto now = std::time(nullptr);
            storage.InsertValue(now, SimulateTemperature(now));
            storage.StoreTemperature();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    if (std::string(argv[2]) == "original") {
        ASSERT(argc == 4);
        std::string portName = argv[3];
        Serial::SerialPort port(portName);
        port.OpenPort();

        while (true) {
            auto data = port.ReadData();

            int time;
            double temp;
            sscanf(data.c_str(), "%d %lf", &time, &temp);
            storage.InsertValue(time, temp);
        }
    }
    
}
