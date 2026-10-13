// Cabin: identity, sensor management and checkSensors(). The state machine itself is in
// test_cabin_state.cpp.
#include "doctest.h"

#include <chrono>
#include <memory>
#include <stdexcept>

#include "cabin/cabin.hpp"
#include "cabin/power_meter.hpp"
#include "cabin/security_sensor.hpp"
#include "cabin/temperature_sensor.hpp"
#include "test_support.hpp"

using namespace std::chrono_literals;
using cabin::Severity;

namespace {

/// A cabin with the scenario's three sensors: 1 temperature, 2 power, 3 security.
std::unique_ptr<cabin::Cabin> equippedCabin()
{
    auto c = std::make_unique<cabin::Cabin>(1, "Fjellhytta", "Hemsedal");
    c->addSensor(std::make_unique<cabin::TemperatureSensor>(1, "Living room"));
    c->addSensor(std::make_unique<cabin::PowerMeter>(2, "Main meter"));
    c->addSensor(std::make_unique<cabin::SecuritySensor>(3, "Front door"));
    return c;
}

/// A sensor kind the cabin does not know, to exercise the generic alert text.
class CustomSensor : public cabin::Sensor
{
public:
    using Sensor::Sensor;
    cabin::Reading read() override
    {
        readings_.emplace_back(test::epoch(), 2.5);
        return readings_.back();
    }
    bool shouldAlert(const cabin::Reading&) const override { return true; }
};

}  // namespace

TEST_CASE("Cabin keeps id, name and location")
{
    const cabin::Cabin c(4, "Sjøbua", "Lofoten");

    CHECK(c.getId() == 4);
    CHECK(c.getName() == "Sjøbua");
    CHECK(c.getLocation() == "Lofoten");
    CHECK(c.getSensors().empty());
}

TEST_CASE("sensors are kept in the order they were added")
{
    auto c = equippedCabin();
    const auto sensors = c->getSensors();

    REQUIRE(sensors.size() == 3);
    CHECK(sensors[0]->getName() == "Living room");
    CHECK(sensors[1]->getName() == "Main meter");
    CHECK(sensors[2]->getName() == "Front door");
}

TEST_CASE("addSensor rejects a null sensor and a duplicate id")
{
    auto c = equippedCabin();

    CHECK_THROWS_AS(c->addSensor(nullptr), std::invalid_argument);
    CHECK_THROWS_AS(c->addSensor(std::make_unique<cabin::SecuritySensor>(2, "Dup")), std::invalid_argument);
    CHECK(c->getSensors().size() == 3);
}

TEST_CASE("removeSensor removes the sensor with that id; an unknown id does nothing")
{
    auto c = equippedCabin();

    c->removeSensor(2);
    const auto sensors = c->getSensors();
    REQUIRE(sensors.size() == 2);
    CHECK(sensors[0]->getId() == 1);
    CHECK(sensors[1]->getId() == 3);

    c->removeSensor(99);
    CHECK(c->getSensors().size() == 2);
}

TEST_CASE_FIXTURE(test::HardwareFixture, "checkSensors without sensors returns no alerts")
{
    cabin::Cabin c(1, "Empty", "Nowhere");

    CHECK(c.checkSensors().empty());
}

TEST_CASE_FIXTURE(test::HardwareFixture, "checkSensors with every value in range returns no alerts")
{
    setSamples({{1, 20.0}, {2, 1.0}, {3, 0.0}});
    auto c = equippedCabin();

    CHECK(c->checkSensors().empty());
    for (const cabin::Sensor* s : c->getSensors())
    {
        CHECK(s->getReadings().size() == 1);   // every sensor was read once
    }
}

TEST_CASE_FIXTURE(test::HardwareFixture, "checkSensors: all three sensors alert (the drawn scenario)")
{
    setSamples({{1, 40.0}, {2, 10.0}, {3, 1.0}});
    auto c = equippedCabin();

    const auto alerts = c->checkSensors();

    REQUIRE(alerts.size() == 3);
    CHECK(alerts[0].getId() == 1);
    CHECK(alerts[0].getSeverity() == Severity::MEDIUM);
    CHECK(alerts[0].getMessage() == "Living room: temperature high (40 °C)");
    CHECK(alerts[0].getTime() == test::epoch());

    CHECK(alerts[1].getId() == 2);
    CHECK(alerts[1].getSeverity() == Severity::LOW);
    CHECK(alerts[1].getMessage() == "Main meter: consumption (10 kWh)");
    CHECK(alerts[1].getTime() == test::epoch() + 1min);

    CHECK(alerts[2].getId() == 3);
    CHECK(alerts[2].getSeverity() == Severity::HIGH);
    CHECK(alerts[2].getMessage() == "Front door: intrusion (1)");
    CHECK(alerts[2].getTime() == test::epoch() + 2min);
}

TEST_CASE_FIXTURE(test::HardwareFixture, "temperature alerts: low, and frost is HIGH")
{
    auto c = equippedCabin();

    setSamples({{1, 2.5}, {2, 1.0}});
    auto alerts = c->checkSensors();
    REQUIRE(alerts.size() == 1);
    CHECK(alerts[0].getSeverity() == Severity::MEDIUM);
    CHECK(alerts[0].getMessage() == "Living room: temperature low (2.5 °C)");

    setSamples({{1, -4.0}, {2, 1.0}});
    alerts = c->checkSensors();
    REQUIRE(alerts.size() == 1);
    CHECK(alerts[0].getSeverity() == Severity::HIGH);
    CHECK(alerts[0].getMessage() == "Living room: temperature low (-4 °C)");
}

TEST_CASE_FIXTURE(test::HardwareFixture, "alert ids continue across checks and are per cabin")
{
    setSamples({{1, 40.0}, {2, 10.0}, {3, 1.0}});
    auto first = equippedCabin();
    auto second = equippedCabin();

    first->checkSensors();
    const auto again = first->checkSensors();
    const auto other = second->checkSensors();

    REQUIRE(again.size() == 3);
    CHECK(again[0].getId() == 4);
    CHECK(again[2].getId() == 6);
    REQUIRE(other.size() == 3);
    CHECK(other[0].getId() == 1);
}

TEST_CASE_FIXTURE(test::HardwareFixture, "a new cabin starts its alert ids at 1, even at a reused address")
{
    setSamples({{3, 1.0}});
    for (int round = 0; round < 3; ++round)
    {
        cabin::Cabin c(1, "Hut", "Here");
        c.addSensor(std::make_unique<cabin::SecuritySensor>(3, "Door"));
        const auto alerts = c.checkSensors();
        REQUIRE(alerts.size() == 1);
        CHECK(alerts[0].getId() == 1);
    }
}

TEST_CASE("an unknown sensor kind gets a generic MEDIUM alert")
{
    cabin::Cabin c(1, "Hut", "Here");
    c.addSensor(std::make_unique<CustomSensor>(9, "Gauge"));

    const auto alerts = c.checkSensors();

    REQUIRE(alerts.size() == 1);
    CHECK(alerts[0].getSeverity() == Severity::MEDIUM);
    CHECK(alerts[0].getMessage() == "Gauge: out of range (2.5)");
}
