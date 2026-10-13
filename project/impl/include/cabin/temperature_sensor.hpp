#pragma once

#include <string>

#include "cabin/sensor.hpp"

namespace cabin {

/// Measures the temperature in °C. Alerts when a reading is below the minimum or above
/// the maximum threshold; a value exactly at a threshold does not alert.
class TemperatureSensor : public Sensor
{
public:
    /// @throws std::invalid_argument if @p minThreshold > @p maxThreshold
    TemperatureSensor(int id, std::string name, double minThreshold = 5.0,
                      double maxThreshold = 30.0);

    Reading read() override;
    bool shouldAlert(const Reading& r) const override;

    /// Sets the allowed range [min, max] in °C.
    /// @throws std::invalid_argument if @p min > @p max (the thresholds are unchanged)
    void setThreshold(double min, double max);

private:
    // Cabin::checkSensors() reads the thresholds to tell "high" from "low" (ADR-0011).
    friend class Cabin;

    double minThreshold_;
    double maxThreshold_;
};

}  // namespace cabin
