// Behaviour every sensor shares through the abstract Sensor: identity and reading history.
#include "doctest.h"

#include <chrono>

#include "cabin/security_sensor.hpp"
#include "test_support.hpp"

using namespace std::chrono_literals;

TEST_CASE_FIXTURE(test::HardwareFixture, "a sensor keeps its id and name")
{
    const cabin::SecuritySensor s(3, "Front door");

    CHECK(s.getId() == 3);
    CHECK(s.getName() == "Front door");
    CHECK(s.getReadings().empty());
}

TEST_CASE_FIXTURE(test::HardwareFixture, "read() samples the sensor's own id, stamps the time and stores the reading")
{
    setSamples({{3, 1.0}, {4, 0.0}});
    cabin::SecuritySensor s(3, "Front door");

    const cabin::Reading r = s.read();

    CHECK(r.getValue() == 1.0);
    CHECK(r.getTime() == test::epoch());
    REQUIRE(s.getReadings().size() == 1);
    CHECK(s.getReadings().front().getValue() == 1.0);
}

TEST_CASE_FIXTURE(test::HardwareFixture, "readings are stored oldest first")
{
    cabin::SecuritySensor s(1, "Hall");
    s.read();
    s.read();

    REQUIRE(s.getReadings().size() == 2);
    CHECK(s.getReadings()[0].getTime() == test::epoch());
    CHECK(s.getReadings()[1].getTime() == test::epoch() + 1min);
}

TEST_CASE_FIXTURE(test::HardwareFixture, "the history keeps the newest 1000 readings")
{
    cabin::SecuritySensor s(1, "Hall");
    for (int i = 0; i < 1000; ++i)
    {
        s.read();
    }
    CHECK(s.getReadings().size() == 1000);
    CHECK(s.getReadings().front().getTime() == test::epoch());

    s.read();   // the 1001st reading evicts the oldest

    CHECK(s.getReadings().size() == 1000);
    CHECK(s.getReadings().front().getTime() == test::epoch() + 1min);
    CHECK(s.getReadings().back().getTime() == test::epoch() + 1000min);
}
