#include "doctest.h"

#include "cabin/security_sensor.hpp"
#include "test_support.hpp"

namespace {

cabin::Reading at(double value) { return cabin::Reading(test::epoch(), value); }

}  // namespace

TEST_CASE("0 is quiet; any other value is an intrusion")
{
    const cabin::SecuritySensor s(3, "Front door");

    CHECK_FALSE(s.shouldAlert(at(0.0)));
    CHECK(s.shouldAlert(at(1.0)));
    CHECK(s.shouldAlert(at(-1.0)));
}

TEST_CASE_FIXTURE(test::HardwareFixture, "read() samples this sensor")
{
    setSamples({{3, 1.0}});
    cabin::SecuritySensor s(3, "Front door");

    CHECK(s.read().getValue() == 1.0);
}
