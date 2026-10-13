#include "cabin/temperature_sensor.hpp"

#include <stdexcept>
#include <utility>

#include "reading_history.hpp"

namespace cabin {

TemperatureSensor::TemperatureSensor(int id, std::string name, double minThreshold,
                                     double maxThreshold)
    : Sensor(id, std::move(name)), minThreshold_(minThreshold), maxThreshold_(maxThreshold)
{
    if (minThreshold > maxThreshold)
    {
        throw std::invalid_argument("TemperatureSensor: minThreshold > maxThreshold");
    }
}

Reading TemperatureSensor::read() { return detail::takeReading(readings_, id_); }

bool TemperatureSensor::shouldAlert(const Reading& r) const
{
    return r.value_ < minThreshold_ || r.value_ > maxThreshold_;
}

void TemperatureSensor::setThreshold(double min, double max)
{
    if (min > max)
    {
        throw std::invalid_argument("TemperatureSensor::setThreshold: min > max");
    }
    minThreshold_ = min;
    maxThreshold_ = max;
}

}  // namespace cabin
