#include <cache/file.h>

#include <ctime>
#include <deque>
#include <mutex>

namespace TemperatureStorage {

class TemperatureStorage {
public:
    TemperatureStorage(const std::filesystem::path& logDir);
    ~TemperatureStorage();

    void InsertValue(time_t time, double value);

    void StoreTemperature();

private:
    std::deque<std::pair<time_t, double>> Readings_;
    File::File ReadingsFile_;
    bool ReadingsToUpdate_;

    std::deque<std::pair<time_t, double>> HourlyAverageReadings_;
    File::File HourlyAverageReadingsFile_;
    bool HourlyAverageReadingsToUpdate_;

    std::deque<std::pair<time_t, double>> DailyAverageReadings_;
    File::File DailyAverageReadingsFile_;
    bool DailyAverageReadingsToUpdate_;

    std::mutex Lock_;
    
};

}
