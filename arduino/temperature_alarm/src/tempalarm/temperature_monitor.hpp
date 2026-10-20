#pragma once

#include <stdint.h>

#include "buzzer.hpp"
#include "interval_timer.hpp"
#include "lcd_display.hpp"
#include "message_formatter.hpp"
#include "serial_link.hpp"
#include "temperature_sensor.hpp"
#include "threshold.hpp"

namespace tempalarm {

/// Measures the temperature once per interval, sounds the buzzer while it is outside the
/// threshold, shows every measurement on the LCD and sends it to the website.
///
/// Its life cycle is the state machine of diagrams/input/state-temperature_monitor.drawio:
/// the buzzer is on exactly while the monitor is Alarming.
class TemperatureMonitor
{
public:
    // The states and events of diagrams/input/state-temperature_monitor.drawio.
    enum class State { Normal, Alarming, SensorFault };
    enum class Event { TemperatureOutOfRange, SensorFailed, TemperatureOk };

    TemperatureMonitor(TemperatureSensor& sensor, Threshold& threshold, Buzzer& buzzer,
                       LcdDisplay& display, SerialLink& link, uint32_t intervalMs);

    TemperatureMonitor(const TemperatureMonitor&) = delete;
    TemperatureMonitor& operator=(const TemperatureMonitor&) = delete;

    /// Call as often as possible with the milliseconds since start. When a measurement is
    /// due: reads the sensor, fires the event its status means, and reports the reading.
    void update(uint32_t nowMs);

    /// Fires @p event: takes the transition of the table that leaves the current state on
    /// @p event. An event with no transition is ignored.
    void handle(Event event);
    State state() const;

private:
    // Actions of the state machine.
    void startAlarm();
    void stopAlarm();

    TemperatureSensor& sensor_;     // TemperatureMonitor --> TemperatureSensor : sensor
    Threshold& threshold_;          // TemperatureMonitor --> Threshold : threshold
    Buzzer& buzzer_;                // TemperatureMonitor --> Buzzer : buzzer
    LcdDisplay& display_;           // TemperatureMonitor --> LcdDisplay : display
    SerialLink& link_;              // TemperatureMonitor --> SerialLink : link
    IntervalTimer timer_;           // TemperatureMonitor *-- IntervalTimer : timer
    MessageFormatter formatter_;    // TemperatureMonitor *-- MessageFormatter : formatter

    State state_ = State::Normal;   // the initial state

    // The transition table: one row per arrow in the state diagram.
    struct Transition
    {
        State from;
        Event event;
        State to;
        bool (TemperatureMonitor::*guard)() const;   // nullptr: no guard
        void (TemperatureMonitor::*action)();        // nullptr: no action
    };
    static constexpr Transition transitions[] = {
        {State::Normal,      Event::TemperatureOutOfRange, State::Alarming,    nullptr, &TemperatureMonitor::startAlarm},
        {State::Normal,      Event::SensorFailed,          State::SensorFault, nullptr, nullptr},
        {State::Alarming,    Event::TemperatureOk,         State::Normal,      nullptr, &TemperatureMonitor::stopAlarm},
        {State::Alarming,    Event::SensorFailed,          State::SensorFault, nullptr, &TemperatureMonitor::stopAlarm},
        {State::SensorFault, Event::TemperatureOk,         State::Normal,      nullptr, nullptr},
        {State::SensorFault, Event::TemperatureOutOfRange, State::Alarming,    nullptr, &TemperatureMonitor::startAlarm},
    };
};

}  // namespace tempalarm
