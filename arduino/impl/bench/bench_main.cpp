// Benchmarks of the hot paths, on the host with the simulated board.
//
//     tempalarm_bench           a table
//     tempalarm_bench --json    {"name": ns per operation, ...}, read by tools/fom.py
//
// Each benchmark runs `repetitions` batches of `batch` operations and reports the median
// batch's time per operation, which is stable against the odd slow batch. Host numbers
// are not Arduino numbers (16 MHz, no FPU): they track relative change between iterations.
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <string>
#include <vector>

#include "tempalarm/command_handler.hpp"
#include "tempalarm/hal_sim.hpp"
#include "tempalarm/temperature_monitor.hpp"

namespace {

namespace sim = tempalarm::hal::sim;

const int repetitions = 15;
const int batch = 20000;

volatile unsigned sink = 0;   // results are written here so the work cannot be optimised away

double nanosecondsPerOperation(const std::function<void()>& operation)
{
    std::vector<double> perOp;
    for (int r = 0; r < repetitions; ++r)
    {
        const auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < batch; ++i)
        {
            operation();
        }
        const auto stop = std::chrono::steady_clock::now();
        perOp.push_back(std::chrono::duration<double, std::nano>(stop - start).count() / batch);
    }
    std::sort(perOp.begin(), perOp.end());
    return perOp[perOp.size() / 2];
}

}  // namespace

int main(int argc, char** argv)
{
    const bool json = argc > 1 && std::strcmp(argv[1], "--json") == 0;
    sim::reset();
    sim::keepSent(false);

    tempalarm::Threshold threshold(18.0f, 28.0f);
    tempalarm::TemperatureSensor sensor(2);
    tempalarm::Buzzer buzzer(8);
    tempalarm::LcdDisplay display(16);
    tempalarm::SerialLink link;
    tempalarm::TemperatureMonitor monitor(sensor, threshold, buzzer, display, link, 0);
    tempalarm::CommandHandler commands(link, threshold);
    tempalarm::CommandParser parser;
    tempalarm::MessageFormatter formatter;

    // A temperature that crosses the threshold every few readings, so update() exercises
    // both state changes and the LCD and serial output every time.
    const float temperatures[] = {22.4f, 29.3f, 30.1f, 16.8f, 21.0f, 27.9f};
    unsigned n = 0;
    std::uint32_t now = 0;

    struct Result { const char* name; double ns; };
    const std::vector<Result> results = {
        {"monitor_update", nanosecondsPerOperation([&] {
             sim::setCelsius(temperatures[n++ % 6]);
             monitor.update(now += 2000);
             sink = sink + static_cast<unsigned>(monitor.state());
         })},
        {"classify", nanosecondsPerOperation([&] {
             sink = sink + static_cast<unsigned>(threshold.classify(temperatures[n++ % 6]));
         })},
        {"parse_command", nanosecondsPerOperation([&] {
             sink = sink + static_cast<unsigned>(parser.parse("SET,-12.5,30.0").kind);
         })},
        {"format_reading", nanosecondsPerOperation([&] {
             sink = sink + static_cast<unsigned char>(
                 formatter.formatReading(temperatures[n++ % 6], tempalarm::TemperatureStatus::Ok, threshold)[5]);
         })},
        {"handle_command", nanosecondsPerOperation([&] {
             sim::receive("SET,18.0,28.0\n");
             commands.poll();
         })},
    };

    if (json)
    {
        std::printf("{");
        for (std::size_t i = 0; i < results.size(); ++i)
        {
            std::printf("%s\"%s\": %.1f", i ? ", " : "", results[i].name, results[i].ns);
        }
        std::printf("}\n");
    }
    else
    {
        std::printf("%-16s %12s\n", "benchmark", "ns / op");
        for (const Result& r : results)
        {
            std::printf("%-16s %12.1f\n", r.name, r.ns);
        }
    }
    return 0;
}
