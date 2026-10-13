#include "cabin/security_sensor.hpp"

#include <utility>

#include "reading_history.hpp"

namespace cabin {

SecuritySensor::SecuritySensor(int id, std::string name) : Sensor(id, std::move(name)) {}

Reading SecuritySensor::read() { return detail::takeReading(readings_, id_); }

bool SecuritySensor::shouldAlert(const Reading& r) const { return r.value_ != 0.0; }

}  // namespace cabin
