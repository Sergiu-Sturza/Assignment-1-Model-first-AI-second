#pragma once

#include "temperature_status.hpp"

namespace tempalarm {

/// The allowed temperature range [low, high] in °C, and what a measurement means
/// against it. A value exactly at a limit is still Ok.
class Threshold
{
public:
    /// Constructs the range as given; use set() for values that need checking.
    Threshold(float low, float high);

    float getLow() const;
    float getHigh() const;

    /// Sets the range when -40 <= low < high <= 125 (°C); otherwise leaves it unchanged.
    /// @return whether the range was changed
    bool set(float low, float high);

    /// TooLow below the range, TooHigh above it, SensorError for NaN (a failed reading).
    TemperatureStatus classify(float celsius) const;

private:
    float low_;
    float high_;
};

}  // namespace tempalarm
