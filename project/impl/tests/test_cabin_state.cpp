// The state machine of diagrams/input/state-cabin.drawio: one test per arrow, each guard
// true and false, and events without an arrow.
#include "doctest.h"

#include <memory>

#include "cabin/cabin.hpp"
#include "cabin/security_sensor.hpp"
#include "test_support.hpp"

using State = cabin::Cabin::State;
using Event = cabin::Cabin::Event;

namespace {

constexpr int doorId = 3;

/// A cabin in Monitoring with one security sensor.
std::unique_ptr<cabin::Cabin> monitoringCabin()
{
    auto c = std::make_unique<cabin::Cabin>(1, "Hut", "Here");
    c->addSensor(std::make_unique<cabin::SecuritySensor>(doorId, "Door"));
    return c;
}

/// A cabin in Alerting user: a check found an intrusion (lastCheck = true), and then
/// MeasurementIntervalElapsed is fired once more while in Monitoring.
std::unique_ptr<cabin::Cabin> alertingCabin()
{
    auto c = monitoringCabin();
    test::HardwareFixture::setSamples({{doorId, 1.0}});
    c->checkSensors();
    c->handle(Event::MeasurementIntervalElapsed);
    return c;
}

}  // namespace

TEST_CASE("the initial state is Unconfigured")
{
    const cabin::Cabin c(1, "Hut", "Here");
    CHECK(c.state() == State::Unconfigured);
}

TEST_CASE("Unconfigured --addSensor [hasSensors]--> Monitoring")
{
    cabin::Cabin c(1, "Hut", "Here");

    SUBCASE("guard false: no sensors, the event is refused")
    {
        c.handle(Event::addSensor);
        CHECK(c.state() == State::Unconfigured);
    }
    SUBCASE("guard true: addSensor() stores the sensor and fires the event")
    {
        c.addSensor(std::make_unique<cabin::SecuritySensor>(doorId, "Door"));
        CHECK(c.state() == State::Monitoring);
    }
}

TEST_CASE("Monitoring --LastSensorRemoved--> Unconfigured")
{
    auto c = monitoringCabin();
    c->addSensor(std::make_unique<cabin::SecuritySensor>(4, "Window"));

    c->removeSensor(4);   // not the last one: no event
    CHECK(c->state() == State::Monitoring);

    c->removeSensor(doorId);
    CHECK(c->state() == State::Unconfigured);
}

TEST_CASE_FIXTURE(test::HardwareFixture, "Monitoring --MeasurementIntervalElapsed [inRange]--> Monitoring")
{
    setSamples({{doorId, 0.0}});
    auto c = monitoringCabin();

    CHECK(c->checkSensors().empty());
    CHECK(c->state() == State::Monitoring);

    c->handle(Event::MeasurementIntervalElapsed);   // the last check was in range
    CHECK(c->state() == State::Monitoring);
}

TEST_CASE_FIXTURE(test::HardwareFixture, "Monitoring --MeasurementIntervalElapsed [outOfRange]--> Alerting user")
{
    auto c = alertingCabin();
    CHECK(c->state() == State::AlertingUser);
}

TEST_CASE_FIXTURE(test::HardwareFixture, "checkSensors with alerts passes through Alerting user back to Monitoring")
{
    setSamples({{doorId, 1.0}});
    auto c = monitoringCabin();

    CHECK(c->checkSensors().size() == 1);
    CHECK(c->state() == State::Monitoring);
}

TEST_CASE_FIXTURE(test::HardwareFixture, "Alerting user --AlertSent--> Monitoring")
{
    auto c = alertingCabin();
    c->handle(Event::AlertSent);
    CHECK(c->state() == State::Monitoring);
}

TEST_CASE_FIXTURE(test::HardwareFixture, "Alerting user --LastSensorRemoved--> Unconfigured")
{
    auto c = alertingCabin();
    c->removeSensor(doorId);
    CHECK(c->state() == State::Unconfigured);
}

TEST_CASE_FIXTURE(test::HardwareFixture, "events without an arrow are ignored")
{
    SUBCASE("in Unconfigured")
    {
        cabin::Cabin c(1, "Hut", "Here");
        c.handle(Event::LastSensorRemoved);
        c.handle(Event::MeasurementIntervalElapsed);
        c.handle(Event::AlertSent);
        CHECK(c.state() == State::Unconfigured);
    }
    SUBCASE("in Monitoring")
    {
        auto c = monitoringCabin();
        c->handle(Event::addSensor);
        c->handle(Event::AlertSent);
        CHECK(c->state() == State::Monitoring);
    }
    SUBCASE("in Alerting user")
    {
        auto c = alertingCabin();
        c->handle(Event::addSensor);
        c->handle(Event::MeasurementIntervalElapsed);
        CHECK(c->state() == State::AlertingUser);
    }
}
