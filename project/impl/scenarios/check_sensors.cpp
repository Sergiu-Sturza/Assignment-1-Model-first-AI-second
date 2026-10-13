// The scenario of diagrams/input/sequence-check_sensors.drawio.
//
// main() sets the objects up; scenario() makes the calls the diagram shows, and only those
// are recorded. The verifier runs this program in a traced build.
#include <memory>

#include "cabin/cabin.hpp"
#include "cabin/hardware.hpp"
#include "cabin/power_meter.hpp"
#include "cabin/security_sensor.hpp"
#include "cabin/temperature_sensor.hpp"

namespace {

void scenario(cabin::Cabin& hut)
{
    hut.checkSensors();   // User -> Cabin : checkSensors()
}

}  // namespace

int main()
{
    // Every sensor reads out of range, so every shouldAlert(r) returns true, as drawn:
    // 40 °C (above 30), 10 kWh (above 3) and movement.
    cabin::hardware::setSampler([](int sensorId) {
        switch (sensorId)
        {
        case 1: return 40.0;
        case 2: return 10.0;
        default: return 1.0;
        }
    });

    cabin::Cabin hut(1, "Fjellhytta", "Hemsedal");
    hut.addSensor(std::make_unique<cabin::TemperatureSensor>(1, "Living room"));
    hut.addSensor(std::make_unique<cabin::PowerMeter>(2, "Main meter"));
    hut.addSensor(std::make_unique<cabin::SecuritySensor>(3, "Front door"));
    scenario(hut);
    return 0;
}
