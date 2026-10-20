// The temperature alarm on the simulated board: a few measurements across the threshold,
// a sensor failure, and the website changing the threshold. Prints what the LCD shows,
// whether the buzzer sounds, and every line sent to the website.
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <string>

#include "tempalarm/command_handler.hpp"
#include "tempalarm/hal_sim.hpp"
#include "tempalarm/temperature_monitor.hpp"

namespace sim = tempalarm::hal::sim;

int main()
{
    const int sensorPin = 2;
    const int buzzerPin = 8;

    tempalarm::Threshold threshold(18.0f, 28.0f);
    tempalarm::TemperatureSensor sensor(sensorPin);
    tempalarm::Buzzer buzzer(buzzerPin);
    tempalarm::LcdDisplay display(16);
    tempalarm::SerialLink link;
    tempalarm::TemperatureMonitor monitor(sensor, threshold, buzzer, display, link, 2000);
    tempalarm::CommandHandler commands(link, threshold);

    const float temperatures[] = {22.5f, 29.1f, 30.4f, NAN, 25.0f, 16.2f, 21.0f};
    std::uint32_t now = 0;
    for (float celsius : temperatures)
    {
        sim::setCelsius(celsius);
        monitor.update(now);
        std::printf("%6u ms  [%-16s|%-16s]  buzzer %-3s\n", static_cast<unsigned>(now),
                    sim::lcdRow(0).c_str(), sim::lcdRow(1).c_str(), buzzer.isOn() ? "ON" : "off");
        now += 2000;
    }

    sim::receive("SET,15.0,22.0\nGET\nSET,30,20\nHELLO\n");
    for (int i = 0; i < 4; ++i)
    {
        commands.poll();
    }
    monitor.update(now);   // 21.0 °C is still inside the new range

    std::printf("\nSent to the website:\n");
    for (const std::string& line : sim::sent())
    {
        std::printf("  %s\n", line.c_str());
    }
    return 0;
}
