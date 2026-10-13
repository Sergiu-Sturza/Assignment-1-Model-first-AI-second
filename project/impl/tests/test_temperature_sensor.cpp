#include "doctest.h"

#include <stdexcept>

#include "cabin/temperature_sensor.hpp"
#include "test_support.hpp"

namespace {

cabin::Reading at(double value) { return cabin::Reading(test::epoch(), value); }

}  // namespace

TEST_CASE("default limits are 5 to 30 degrees, exclusive")
{
    const cabin::TemperatureSensor t(1, "Living room");

    CHECK(t.shouldAlert(at(4.999)));
    CHECK_FALSE(t.shouldAlert(at(5.0)));
    CHECK_FALSE(t.shouldAlert(at(20.0)));
    CHECK_FALSE(t.shouldAlert(at(30.0)));
    CHECK(t.shouldAlert(at(30.001)));
}

TEST_CASE("constructor limits are used")
{
    const cabin::TemperatureSensor t(1, "Cellar", -2.0, 8.0);

    CHECK(t.shouldAlert(at(-2.5)));
    CHECK_FALSE(t.shouldAlert(at(-2.0)));
    CHECK_FALSE(t.shouldAlert(at(8.0)));
    CHECK(t.shouldAlert(at(8.5)));
}

TEST_CASE("constructor rejects min > max")
{
    CHECK_THROWS_AS(cabin::TemperatureSensor(1, "x", 10.0, 9.0), std::invalid_argument);
}

TEST_CASE("setThreshold changes the limits; min == max is allowed")
{
    cabin::TemperatureSensor t(1, "Living room");
    t.setThreshold(18.0, 18.0);

    CHECK_FALSE(t.shouldAlert(at(18.0)));
    CHECK(t.shouldAlert(at(17.9)));
    CHECK(t.shouldAlert(at(18.1)));
}

TEST_CASE("setThreshold rejects min > max and keeps the old limits")
{
    cabin::TemperatureSensor t(1, "Living room");

    CHECK_THROWS_AS(t.setThreshold(25.0, 15.0), std::invalid_argument);
    CHECK_FALSE(t.shouldAlert(at(5.0)));
    CHECK(t.shouldAlert(at(4.0)));
}

TEST_CASE_FIXTURE(test::HardwareFixture, "read() samples this sensor")
{
    setSamples({{7, 21.5}});
    cabin::TemperatureSensor t(7, "Bedroom");

    CHECK(t.read().getValue() == doctest::Approx(21.5));
    CHECK(t.getReadings().size() == 1);
}
