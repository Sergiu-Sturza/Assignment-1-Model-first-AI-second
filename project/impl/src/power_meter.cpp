#include "cabin/power_meter.hpp"

#include <utility>

#include "reading_history.hpp"

namespace cabin {

PowerMeter::PowerMeter(int id, std::string name, double threshold)
    : Sensor(id, std::move(name)), threshold_(threshold)
{
}

Reading PowerMeter::read() { return detail::takeReading(readings_, id_); }

bool PowerMeter::shouldAlert(const Reading& r) const { return r.value_ > threshold_; }

void PowerMeter::setThreshold(double t) { threshold_ = t; }

}  // namespace cabin
