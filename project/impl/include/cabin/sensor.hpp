#pragma once

#include <string>
#include <vector>

#include "cabin/reading.hpp"

namespace cabin {

/// A sensor in a cabin («abstract»). Each kind decides for itself when a reading must
/// raise an alert.
///
/// Every read() is stored in the sensor's history, which keeps the most recent
/// 1 000 readings (ADR-0009).
class Sensor
{
public:
    /// @param id   unique within the cabin the sensor is added to
    /// @param name shown in alert messages
    Sensor(int id, std::string name);
    virtual ~Sensor() = default;

    Sensor(const Sensor&) = delete;              // polymorphic: no slicing copies
    Sensor& operator=(const Sensor&) = delete;

    int getId() const;
    const std::string& getName() const;

    /// Takes a new measurement, stores it in the history and returns it.
    virtual Reading read() = 0;
    /// True if @p r is outside this sensor's limits and must raise an alert.
    virtual bool shouldAlert(const Reading& r) const = 0;

    /// The stored readings, oldest first.
    const std::vector<Reading>& getReadings() const;

protected:
    std::vector<Reading> readings_;   // Sensor *-- "*" Reading : readings

private:
    // The subclasses' read() and Cabin::checkSensors() read id_ and name_ directly, so
    // that the traced scenario makes exactly the drawn calls (ADR-0011).
    friend class TemperatureSensor;
    friend class PowerMeter;
    friend class SecuritySensor;
    friend class Cabin;

    int id_;
    std::string name_;
};

}  // namespace cabin
