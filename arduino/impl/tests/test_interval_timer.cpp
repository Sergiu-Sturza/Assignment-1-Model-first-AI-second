#include <cstdint>

#include "doctest.h"
#include "tempalarm/interval_timer.hpp"

using tempalarm::IntervalTimer;

TEST_CASE("IntervalTimer: due at the first call, whatever the time")
{
    IntervalTimer timer(2000);
    CHECK(timer.isDue(123456));
}

TEST_CASE("IntervalTimer: due once per interval")
{
    IntervalTimer timer(2000);
    CHECK(timer.isDue(0));
    CHECK_FALSE(timer.isDue(1));
    CHECK_FALSE(timer.isDue(1999));
    CHECK(timer.isDue(2000));
    CHECK_FALSE(timer.isDue(2000));
    CHECK(timer.isDue(4500));        // late: the next interval starts from here
    CHECK_FALSE(timer.isDue(6000));
    CHECK(timer.isDue(6500));
}

TEST_CASE("IntervalTimer: correct when the millisecond counter wraps around")
{
    IntervalTimer timer(2000);
    const std::uint32_t nearEnd = 0xFFFFFFFFu - 500;
    CHECK(timer.isDue(nearEnd));
    CHECK_FALSE(timer.isDue(1000));   // 1501 ms later, across the wrap
    CHECK(timer.isDue(1500));         // 2001 ms later
}
