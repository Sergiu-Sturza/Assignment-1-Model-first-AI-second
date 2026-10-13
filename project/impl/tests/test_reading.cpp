#include "doctest.h"

#include <chrono>

#include "cabin/reading.hpp"
#include "test_support.hpp"

TEST_CASE("Reading keeps its time and value")
{
    const cabin::DateTime t = test::epoch() + std::chrono::minutes{5};
    const cabin::Reading r(t, -3.25);

    CHECK(r.getTime() == t);
    CHECK(r.getValue() == doctest::Approx(-3.25));
}
