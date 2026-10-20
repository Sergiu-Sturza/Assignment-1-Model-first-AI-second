#include <cmath>

#include "doctest.h"
#include "tempalarm/threshold.hpp"

using tempalarm::TemperatureStatus;
using tempalarm::Threshold;

TEST_CASE("Threshold: classify against [low, high], limits included")
{
    const Threshold t(18.0f, 28.0f);
    CHECK(t.classify(22.0f) == TemperatureStatus::Ok);
    CHECK(t.classify(18.0f) == TemperatureStatus::Ok);
    CHECK(t.classify(28.0f) == TemperatureStatus::Ok);
    CHECK(t.classify(17.9f) == TemperatureStatus::TooLow);
    CHECK(t.classify(28.1f) == TemperatureStatus::TooHigh);
    CHECK(t.classify(-40.0f) == TemperatureStatus::TooLow);
}

TEST_CASE("Threshold: a failed reading (NaN) is a sensor error")
{
    const Threshold t(18.0f, 28.0f);
    CHECK(t.classify(NAN) == TemperatureStatus::SensorError);
}

TEST_CASE("Threshold: set accepts a valid range")
{
    Threshold t(18.0f, 28.0f);
    CHECK(t.set(-5.5f, 40.0f));
    CHECK(t.getLow() == -5.5f);
    CHECK(t.getHigh() == 40.0f);
    CHECK(t.set(-40.0f, 125.0f));   // the widest range allowed
}

TEST_CASE("Threshold: set rejects an invalid range and keeps the old one")
{
    Threshold t(18.0f, 28.0f);
    CHECK_FALSE(t.set(30.0f, 20.0f));    // reversed
    CHECK_FALSE(t.set(25.0f, 25.0f));    // empty
    CHECK_FALSE(t.set(-41.0f, 20.0f));   // below what sensors report
    CHECK_FALSE(t.set(20.0f, 126.0f));   // above
    CHECK_FALSE(t.set(NAN, 20.0f));
    CHECK_FALSE(t.set(10.0f, NAN));
    CHECK_FALSE(t.set(-INFINITY, INFINITY));
    CHECK(t.getLow() == 18.0f);
    CHECK(t.getHigh() == 28.0f);
}
