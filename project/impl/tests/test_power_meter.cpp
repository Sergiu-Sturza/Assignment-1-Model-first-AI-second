#include "doctest.h"

#include "cabin/power_meter.hpp"
#include "test_support.hpp"

namespace {

cabin::Reading at(double value) { return cabin::Reading(test::epoch(), value); }

}  // namespace

TEST_CASE("default threshold is 3 kWh, exclusive, upper limit only")
{
    const cabin::PowerMeter p(2, "Main meter");

    CHECK_FALSE(p.shouldAlert(at(0.0)));
    CHECK_FALSE(p.shouldAlert(at(3.0)));
    CHECK(p.shouldAlert(at(3.001)));
}

TEST_CASE("setThreshold changes the limit")
{
    cabin::PowerMeter p(2, "Main meter", 10.0);
    CHECK_FALSE(p.shouldAlert(at(5.0)));

    p.setThreshold(4.0);

    CHECK(p.shouldAlert(at(5.0)));
    CHECK_FALSE(p.shouldAlert(at(4.0)));
}

TEST_CASE_FIXTURE(test::HardwareFixture, "read() samples this meter")
{
    setSamples({{2, 1.25}});
    cabin::PowerMeter p(2, "Main meter");

    CHECK(p.read().getValue() == doctest::Approx(1.25));
    CHECK(p.getReadings().size() == 1);
}
