#include "cabin/hardware.hpp"

#include <utility>

namespace cabin::hardware {

namespace {

using namespace std::chrono;

constexpr DateTime epoch{sys_days{year{2026} / January / 1}};

DateTime simulatedTime = epoch;

double defaultSample(int /*sensorId*/) { return 0.0; }

DateTime defaultNow()
{
    const DateTime t = simulatedTime;
    simulatedTime += minutes{1};
    return t;
}

Sampler& currentSampler()
{
    static Sampler sampler = defaultSample;
    return sampler;
}

Clock& currentClock()
{
    static Clock clock = defaultNow;
    return clock;
}

}  // namespace

double sample(int sensorId) { return currentSampler()(sensorId); }

DateTime now() { return currentClock()(); }

void setSampler(Sampler sampler)
{
    currentSampler() = sampler ? std::move(sampler) : Sampler{defaultSample};
}

void setClock(Clock clock)
{
    currentClock() = clock ? std::move(clock) : Clock{defaultNow};
}

void reset()
{
    setSampler(nullptr);
    setClock(nullptr);
    simulatedTime = epoch;
}

}  // namespace cabin::hardware
