#pragma once

#include <functional>

#include "cabin/date_time.hpp"

/// Where sensor values and time come from (ADR-0006).
///
/// Free functions, not a class, so they are neither part of the class diagram nor
/// messages in a sequence diagram. Tests, scenarios and a real hardware binding replace
/// the defaults with setSampler() and setClock(). Global state, not thread-safe.
namespace cabin::hardware {

/// Produces the current value of the sensor with the given id.
using Sampler = std::function<double(int sensorId)>;
/// Produces the current time.
using Clock = std::function<DateTime()>;

/// The current value of sensor @p sensorId. Default: 0.0 for every sensor.
double sample(int sensorId);
/// The current time. Default: a simulated clock that starts at 2026-01-01T00:00:00Z and
/// advances one minute per call.
DateTime now();

/// Replaces the sampler; an empty function restores the default.
void setSampler(Sampler sampler);
/// Replaces the clock; an empty function restores the default.
void setClock(Clock clock);
/// Restores the default sampler and clock, and restarts the simulated clock.
void reset();

}  // namespace cabin::hardware
