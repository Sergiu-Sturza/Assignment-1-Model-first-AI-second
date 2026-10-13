#pragma once

#include "cabin/date_time.hpp"

namespace cabin {

/// One measured value of a sensor, with the time it was taken.
///
/// Units depend on the sensor: °C for temperature, kWh used in the interval for power,
/// 0 (quiet) or non-zero (movement / door open) for security.
class Reading
{
public:
    Reading(DateTime time, double value);

    /// When the value was measured.
    DateTime getTime() const;
    /// The measured value.
    double getValue() const;

private:
    // The classes on the drawn checkSensors() path read the fields directly, so that the
    // traced scenario makes exactly the drawn calls (ADR-0011).
    friend class TemperatureSensor;
    friend class PowerMeter;
    friend class SecuritySensor;
    friend class Cabin;

    DateTime time_;
    double value_;
};

}  // namespace cabin
