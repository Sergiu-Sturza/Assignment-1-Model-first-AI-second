#include "doctest.h"

#include "cabin/alert.hpp"
#include "test_support.hpp"

TEST_CASE("Alert keeps id, time, severity and message")
{
    const cabin::Alert a(7, test::epoch(), cabin::Severity::HIGH, "Front door: intrusion (1)");

    CHECK(a.getId() == 7);
    CHECK(a.getTime() == test::epoch());
    CHECK(a.getSeverity() == cabin::Severity::HIGH);
    CHECK(a.getMessage() == "Front door: intrusion (1)");
}
