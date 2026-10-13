#pragma once

// Internal to the library: the read() step all sensors share. Lives in src/, not in
// include/, so it is not part of the compared design.

#include <cstddef>
#include <vector>

#include "cabin/hardware.hpp"
#include "cabin/reading.hpp"

namespace cabin::detail {

/// The most readings a sensor keeps (ADR-0009).
inline constexpr std::size_t maxReadings = 1000;

/// Measures sensor @p sensorId now, appends the reading to @p history (dropping the
/// oldest one when the history is full) and returns it.
inline Reading takeReading(std::vector<Reading>& history, int sensorId)
{
    const DateTime time = hardware::now();
    const double value = hardware::sample(sensorId);
    if (history.size() >= maxReadings)
    {
        history.erase(history.begin());
    }
    history.emplace_back(time, value);
    return history.back();
}

}  // namespace cabin::detail
