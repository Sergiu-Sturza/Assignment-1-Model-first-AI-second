// The scenario of diagrams/input/sequence-measure_temperature.drawio.
//
// main() sets the objects up; scenario() makes the calls the diagram shows, and only those
// are recorded. The verifier runs this program in a traced build.
#include "tempalarm/hal_sim.hpp"
#include "tempalarm/temperature_monitor.hpp"

namespace {

void scenario(tempalarm::TemperatureMonitor& monitor)
{
    monitor.update(0);   // Loop -> TemperatureMonitor : update(nowMs)
}

}  // namespace

int main()
{
    // 31.5 °C is above the 28.0 °C limit: the measurement is TooHigh, as drawn.
    tempalarm::hal::sim::setCelsius(31.5f);

    tempalarm::Threshold threshold(18.0f, 28.0f);
    tempalarm::TemperatureSensor sensor(2);
    tempalarm::Buzzer buzzer(8);
    tempalarm::LcdDisplay display(16);
    tempalarm::SerialLink link;
    tempalarm::TemperatureMonitor monitor(sensor, threshold, buzzer, display, link, 2000);
    scenario(monitor);
    return 0;
}
