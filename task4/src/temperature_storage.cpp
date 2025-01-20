#include <temperature_storage.h>

#include <common/public.h>
#include <string.h>

#include <algorithm>

namespace TemperatureStorage {

namespace {

std::pair<time_t, double> ParseLine(const char* line) {
    std::tm tm{};
    double value;
    sscanf(line, "%2i/%2i/%4i - %2i:%2i:%2i %lf",
        &tm.tm_mday,
        &tm.tm_mon,
        &tm.tm_year,
        &tm.tm_hour,
        &tm.tm_min,
        &tm.tm_sec,
        &value);
    tm.tm_mon--;
    tm.tm_year -= 1900;

    std::time_t time = std::mktime(&tm);
    return { time, value };
}

void PrintLine(time_t time, double value, char* output) {
    auto tm = std::localtime(&time);
    sprintf(output, "%.2i/%.2i/%.4i - %.2i:%.2i:%.2i %lf",
        tm->tm_mday,
        tm->tm_mon + 1,
        tm->tm_year + 1900,
        tm->tm_hour,
        tm->tm_min,
        tm->tm_sec,
        value);
    output[strlen(output)] = ' ';
}

std::deque<std::pair<time_t, double>> ParseFile(File::File& file) {
    std::string data;
    file.Read(&data);
    
    char* dt = data.data();

    std::deque<std::pair<time_t, double>> result;
    while (auto token = strtok_r(dt, "\n", &dt)) {
        result.push_back(ParseLine(token));
    }
    return result;
}

void PrintToFile(const std::deque<std::pair<time_t, double>>& data, File::File& file) {
    std::string result(data.size() * 40, ' ');
    size_t position = 0;
    for (const auto& [time, value] : data) {
        PrintLine(time, value, result.data() + position * 40);
        position++;
        result[position * 40 - 1] = '\n';
    }
    file.Write(result, "w");
}

double GetAverageTempSince(time_t since, const std::deque<std::pair<time_t, double>>& data) {
    auto begin = std::lower_bound(data.begin(), data.end(), std::make_pair(since, -100000.0));
    double sum = 0;
    size_t count = 0;
    for (; begin != data.end(); begin++) {
        sum += begin->second;
        count++;
    }

    VERIFY(count);
    return sum / count;
}

}

TemperatureStorage::TemperatureStorage(const std::filesystem::path& logDir)
    : ReadingsFile_(logDir / "temperature.log"),
      HourlyAverageReadingsFile_(logDir / "hourly_average.log"),
      DailyAverageReadingsFile_(logDir / "daily_average.log")
{ 
    Readings_ = ParseFile(ReadingsFile_);
    HourlyAverageReadings_ = ParseFile(HourlyAverageReadingsFile_);
    DailyAverageReadings_ = ParseFile(DailyAverageReadingsFile_);
}

TemperatureStorage::~TemperatureStorage() {
    StoreTemperature();
}

void TemperatureStorage::InsertValue(time_t time, double value) {
    static time_t HOUR = 3600;
    static time_t DAY = 24 * HOUR;
    static time_t MONTH = 30 * HOUR;
    static time_t YEAR = 12 * MONTH;

    auto guard = std::lock_guard(Lock_);
    ASSERT(Readings_.empty() || time >= Readings_.back().first);
    Readings_.emplace_back(time, value);
    ReadingsToUpdate_ = true;

    if (HourlyAverageReadings_.empty() && time - Readings_.front().first >= HOUR
        || !HourlyAverageReadings_.empty() && time - HourlyAverageReadings_.back().first >= HOUR)
    {
        HourlyAverageReadings_.emplace_back(time, GetAverageTempSince(time - HOUR, Readings_));
        HourlyAverageReadingsToUpdate_ = true;
    }

    if (HourlyAverageReadings_.empty()) {
        return;
    }

    if (DailyAverageReadings_.empty() && time - HourlyAverageReadings_.front().first >= DAY
        || !DailyAverageReadings_.empty() && time - DailyAverageReadings_.back().first >= DAY)
    {
        DailyAverageReadings_.emplace_back(time, GetAverageTempSince(time - DAY, HourlyAverageReadings_));
        DailyAverageReadingsToUpdate_ = true;
    }

    while (!Readings_.empty() && Readings_.front().first < time - DAY) {
        Readings_.pop_front();
        ReadingsToUpdate_ = true;
    }

    while (!HourlyAverageReadings_.empty() && HourlyAverageReadings_.front().first < HourlyAverageReadings_.back().first - MONTH) {
        HourlyAverageReadings_.pop_front();
        HourlyAverageReadingsToUpdate_ = true;
    }

    while (!DailyAverageReadings_.empty() && DailyAverageReadings_.front().first < DailyAverageReadings_.back().first - YEAR) {
        DailyAverageReadings_.pop_front();
        DailyAverageReadingsToUpdate_ = true;
    }
}

void TemperatureStorage::StoreTemperature() {
    auto guard = std::lock_guard(Lock_);
    if (ReadingsToUpdate_) {
        PrintToFile(Readings_, ReadingsFile_);
    }
    if (HourlyAverageReadingsToUpdate_) {
        PrintToFile(HourlyAverageReadings_, HourlyAverageReadingsFile_);
    }
    if (DailyAverageReadingsToUpdate_) {
        PrintToFile(DailyAverageReadings_, DailyAverageReadingsFile_);
    }
}

}
