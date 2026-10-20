#include "tempalarm/threshold.hpp"

#include "number_text.hpp"

namespace tempalarm {

namespace {

// The range set() accepts, in °C: what common hobby sensors (DHT11/22, DS18B20) report.
const float lowest = -40.0f;
const float highest = 125.0f;

}  // namespace

Threshold::Threshold(float low, float high) : low_(low), high_(high) {}

float Threshold::getLow() const { return low_; }

float Threshold::getHigh() const { return high_; }

bool Threshold::set(float low, float high)
{
    // Written so that NaN fails every comparison and is rejected.
    if (!(low >= lowest && high <= highest && low < high))
    {
        return false;
    }
    low_ = low;
    high_ = high;
    return true;
}

TemperatureStatus Threshold::classify(float celsius) const
{
    if (detail::isNotANumber(celsius))
    {
        return TemperatureStatus::SensorError;
    }
    if (celsius < low_)
    {
        return TemperatureStatus::TooLow;
    }
    if (celsius > high_)
    {
        return TemperatureStatus::TooHigh;
    }
    return TemperatureStatus::Ok;
}

}  // namespace tempalarm
