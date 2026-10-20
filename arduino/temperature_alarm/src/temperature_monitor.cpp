#include "tempalarm/temperature_monitor.hpp"

namespace tempalarm {

#if __cplusplus < 201703L
// Before C++17 (avr-gcc builds the sketch as C++11) a constexpr static member that is
// used needs a definition outside the class.
constexpr TemperatureMonitor::Transition TemperatureMonitor::transitions[];
#endif

namespace {

/// The event a measurement's status fires.
TemperatureMonitor::Event eventFor(TemperatureStatus status)
{
    switch (status)
    {
    case TemperatureStatus::Ok: return TemperatureMonitor::Event::TemperatureOk;
    case TemperatureStatus::TooLow:
    case TemperatureStatus::TooHigh: return TemperatureMonitor::Event::TemperatureOutOfRange;
    case TemperatureStatus::SensorError: break;
    }
    return TemperatureMonitor::Event::SensorFailed;
}

}  // namespace

TemperatureMonitor::TemperatureMonitor(TemperatureSensor& sensor, Threshold& threshold,
                                       Buzzer& buzzer, LcdDisplay& display, SerialLink& link,
                                       uint32_t intervalMs)
    : sensor_(sensor), threshold_(threshold), buzzer_(buzzer), display_(display), link_(link),
      timer_(intervalMs)
{
}

void TemperatureMonitor::update(uint32_t nowMs)
{
    if (!timer_.isDue(nowMs))
    {
        return;
    }
    const float celsius = sensor_.read();
    const TemperatureStatus status = threshold_.classify(celsius);
    handle(eventFor(status));
    if (status == TemperatureStatus::SensorError)
    {
        display_.showSensorError();
    }
    else
    {
        display_.showTemperature(celsius, status);
    }
    link_.writeLine(formatter_.formatReading(celsius, status, threshold_));
}

void TemperatureMonitor::handle(Event event)
{
    for (const Transition& t : transitions)
    {
        if (t.from != state_ || t.event != event)
        {
            continue;
        }
        if (t.guard != nullptr && !(this->*t.guard)())
        {
            continue;
        }
        if (t.action != nullptr)
        {
            (this->*t.action)();
        }
        state_ = t.to;
        return;
    }
    // No transition for this event in this state: the event is ignored.
}

TemperatureMonitor::State TemperatureMonitor::state() const { return state_; }

void TemperatureMonitor::startAlarm() { buzzer_.turnOn(); }

void TemperatureMonitor::stopAlarm() { buzzer_.turnOff(); }

}  // namespace tempalarm
