// A small walk through the cabin monitor: one user, one cabin with three sensors, three
// measurement intervals. The alerts of each interval are delivered to the user.
#include <iostream>
#include <memory>

#include "cabin/cabin.hpp"
#include "cabin/hardware.hpp"
#include "cabin/power_meter.hpp"
#include "cabin/security_sensor.hpp"
#include "cabin/temperature_sensor.hpp"
#include "cabin/user.hpp"

namespace {

const char* toString(cabin::Cabin::State s)
{
    switch (s)
    {
    case cabin::Cabin::State::Unconfigured: return "Unconfigured";
    case cabin::Cabin::State::Monitoring:   return "Monitoring";
    case cabin::Cabin::State::AlertingUser: return "Alerting user";
    }
    return "?";
}

const char* toString(cabin::Severity s)
{
    switch (s)
    {
    case cabin::Severity::LOW:    return "LOW";
    case cabin::Severity::MEDIUM: return "MEDIUM";
    case cabin::Severity::HIGH:   return "HIGH";
    }
    return "?";
}

}  // namespace

int main()
{
    cabin::User user(1, "Kari");
    user.addCabin(std::make_unique<cabin::Cabin>(10, "Fjellhytta", "Hemsedal"));
    cabin::Cabin& hut = *user.getCabins().front();
    std::cout << "state: " << toString(hut.state()) << '\n';

    hut.addSensor(std::make_unique<cabin::TemperatureSensor>(1, "Living room"));
    hut.addSensor(std::make_unique<cabin::PowerMeter>(2, "Main meter"));
    hut.addSensor(std::make_unique<cabin::SecuritySensor>(3, "Front door"));
    std::cout << "state: " << toString(hut.state()) << '\n';

    // Interval 1: all quiet. Interval 2: frost. Interval 3: high use and a door opening.
    const double values[3][3] = {{20.0, 1.5, 0.0}, {-4.0, 2.0, 0.0}, {18.0, 7.5, 1.0}};
    for (const auto& interval : values)
    {
        cabin::hardware::setSampler([&interval](int id) { return interval[id - 1]; });
        for (const cabin::Alert& a : hut.checkSensors())
        {
            user.addAlert(a);
        }
        std::cout << "state: " << toString(hut.state()) << '\n';
    }

    for (const cabin::Alert& a : user.getAlerts())
    {
        std::cout << "alert " << a.getId() << " [" << toString(a.getSeverity()) << "] "
                  << a.getMessage() << '\n';
    }
    return 0;
}
