#include "doctest.h"

#include <chrono>

#include "cabin/hardware.hpp"
#include "test_support.hpp"

using namespace std::chrono_literals;

TEST_CASE_FIXTURE(test::HardwareFixture, "default sampler reports 0 for every sensor")
{
    CHECK(cabin::hardware::sample(1) == 0.0);
    CHECK(cabin::hardware::sample(42) == 0.0);
}

TEST_CASE_FIXTURE(test::HardwareFixture, "default clock starts at the epoch and advances one minute per call")
{
    CHECK(cabin::hardware::now() == test::epoch());
    CHECK(cabin::hardware::now() == test::epoch() + 1min);
    CHECK(cabin::hardware::now() == test::epoch() + 2min);
}

TEST_CASE_FIXTURE(test::HardwareFixture, "a custom sampler and clock replace the defaults")
{
    cabin::hardware::setSampler([](int id) { return id * 1.5; });
    cabin::hardware::setClock([] { return test::epoch() + 24h; });

    CHECK(cabin::hardware::sample(2) == doctest::Approx(3.0));
    CHECK(cabin::hardware::now() == test::epoch() + 24h);
}

TEST_CASE_FIXTURE(test::HardwareFixture, "an empty function restores the default")
{
    cabin::hardware::setSampler([](int) { return 9.0; });
    cabin::hardware::setClock([] { return test::epoch() + 1h; });
    cabin::hardware::setSampler(nullptr);
    cabin::hardware::setClock(nullptr);

    CHECK(cabin::hardware::sample(1) == 0.0);
    CHECK(cabin::hardware::now() == test::epoch());
}

TEST_CASE_FIXTURE(test::HardwareFixture, "reset restarts the simulated clock")
{
    cabin::hardware::now();
    cabin::hardware::now();
    cabin::hardware::reset();

    CHECK(cabin::hardware::now() == test::epoch());
}
