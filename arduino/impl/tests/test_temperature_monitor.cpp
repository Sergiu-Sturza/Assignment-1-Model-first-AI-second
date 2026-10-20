#include <cmath>
#include <cstdint>
#include <string>

#include "board.hpp"
#include "doctest.h"
#include "tempalarm/temperature_monitor.hpp"

namespace sim = tempalarm::hal::sim;
using tempalarm::TemperatureMonitor;
using State = TemperatureMonitor::State;
using Event = TemperatureMonitor::Event;

namespace {

/// The sketch's objects on the simulated board, measuring every 2 s.
struct Rig : test::FreshBoard
{
    tempalarm::Threshold threshold{18.0f, 28.0f};
    tempalarm::TemperatureSensor sensor{test::sensorPin};
    tempalarm::Buzzer buzzer{test::buzzerPin};
    tempalarm::LcdDisplay display{16};
    tempalarm::SerialLink link;
    TemperatureMonitor monitor{sensor, threshold, buzzer, display, link, 2000};
    std::uint32_t now = 0;

    /// One measurement at @p celsius, 2 s after the previous one.
    void measure(float celsius)
    {
        sim::setCelsius(celsius);
        monitor.update(now);
        now += 2000;
    }
};

}  // namespace

// ---- the state machine, one test per arrow of state-temperature_monitor.drawio

TEST_CASE_FIXTURE(Rig, "state machine: starts Normal, buzzer off")
{
    CHECK(monitor.state() == State::Normal);
    CHECK_FALSE(buzzer.isOn());
}

TEST_CASE_FIXTURE(Rig, "state machine: Normal -> Alarming on TemperatureOutOfRange / startAlarm")
{
    monitor.handle(Event::TemperatureOutOfRange);
    CHECK(monitor.state() == State::Alarming);
    CHECK(buzzer.isOn());
}

TEST_CASE_FIXTURE(Rig, "state machine: Normal -> SensorFault on SensorFailed")
{
    monitor.handle(Event::SensorFailed);
    CHECK(monitor.state() == State::SensorFault);
    CHECK_FALSE(buzzer.isOn());
}

TEST_CASE_FIXTURE(Rig, "state machine: Alarming -> Normal on TemperatureOk / stopAlarm")
{
    monitor.handle(Event::TemperatureOutOfRange);
    monitor.handle(Event::TemperatureOk);
    CHECK(monitor.state() == State::Normal);
    CHECK_FALSE(buzzer.isOn());
}

TEST_CASE_FIXTURE(Rig, "state machine: Alarming -> SensorFault on SensorFailed / stopAlarm")
{
    monitor.handle(Event::TemperatureOutOfRange);
    monitor.handle(Event::SensorFailed);
    CHECK(monitor.state() == State::SensorFault);
    CHECK_FALSE(buzzer.isOn());
}

TEST_CASE_FIXTURE(Rig, "state machine: SensorFault -> Normal on TemperatureOk")
{
    monitor.handle(Event::SensorFailed);
    monitor.handle(Event::TemperatureOk);
    CHECK(monitor.state() == State::Normal);
    CHECK_FALSE(buzzer.isOn());
}

TEST_CASE_FIXTURE(Rig, "state machine: SensorFault -> Alarming on TemperatureOutOfRange / startAlarm")
{
    monitor.handle(Event::SensorFailed);
    monitor.handle(Event::TemperatureOutOfRange);
    CHECK(monitor.state() == State::Alarming);
    CHECK(buzzer.isOn());
}

TEST_CASE_FIXTURE(Rig, "state machine: events without an arrow are ignored")
{
    monitor.handle(Event::TemperatureOk);                // Normal has no TemperatureOk arrow
    CHECK(monitor.state() == State::Normal);
    monitor.handle(Event::TemperatureOutOfRange);
    monitor.handle(Event::TemperatureOutOfRange);        // nor has Alarming
    CHECK(monitor.state() == State::Alarming);
    CHECK(buzzer.isOn());
    monitor.handle(Event::SensorFailed);
    monitor.handle(Event::SensorFailed);                 // nor has SensorFault
    CHECK(monitor.state() == State::SensorFault);
}

// ---- update(): the whole measurement cycle

TEST_CASE_FIXTURE(Rig, "update: too hot sounds the buzzer until the temperature is back in range")
{
    measure(22.0f);
    CHECK_FALSE(sim::pin(test::buzzerPin));
    measure(28.5f);
    CHECK(sim::pin(test::buzzerPin));
    CHECK(sim::lcdRow(1) == "ALARM: TOO HIGH ");
    measure(28.0f);                                      // at the limit is in range
    CHECK_FALSE(sim::pin(test::buzzerPin));
    CHECK(sim::lcdRow(1) == "Status: OK      ");
}

TEST_CASE_FIXTURE(Rig, "update: too cold sounds the buzzer too")
{
    measure(12.0f);
    CHECK(monitor.state() == State::Alarming);
    CHECK(sim::pin(test::buzzerPin));
    CHECK(sim::lcdRow(1) == "ALARM: TOO LOW  ");
}

TEST_CASE_FIXTURE(Rig, "update: a failed sensor is shown and silences the buzzer")
{
    measure(35.0f);
    measure(NAN);
    CHECK(monitor.state() == State::SensorFault);
    CHECK_FALSE(sim::pin(test::buzzerPin));
    CHECK(sim::lcdRow(0) == "Sensor error    ");
    CHECK(sim::sent().back() == "T,,ERROR,18.0,28.0");
}

TEST_CASE_FIXTURE(Rig, "update: every measurement is sent to the website, nothing in between")
{
    measure(21.04f);
    monitor.update(now - 1000);                          // not due: nothing happens
    measure(30.0f);
    REQUIRE(sim::sent().size() == 2);
    CHECK(sim::sent()[0] == "T,21.0,OK,18.0,28.0");
    CHECK(sim::sent()[1] == "T,30.0,HIGH,18.0,28.0");
}

TEST_CASE_FIXTURE(Rig, "update: a changed threshold applies from the next measurement")
{
    measure(25.0f);
    CHECK_FALSE(buzzer.isOn());
    REQUIRE(threshold.set(15.0f, 22.0f));
    measure(25.0f);
    CHECK(buzzer.isOn());
    CHECK(sim::sent().back() == "T,25.0,HIGH,15.0,22.0");
}
