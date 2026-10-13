#include "doctest.h"

#include <memory>
#include <stdexcept>

#include "cabin/security_sensor.hpp"
#include "cabin/user.hpp"
#include "test_support.hpp"

TEST_CASE("User keeps id and name, and starts with no cabins or alerts")
{
    const cabin::User u(1, "Kari");

    CHECK(u.getId() == 1);
    CHECK(u.getName() == "Kari");
    CHECK(u.getCabins().empty());
    CHECK(u.getAlerts().empty());
}

TEST_CASE("cabins are kept in the order they were added")
{
    cabin::User u(1, "Kari");
    u.addCabin(std::make_unique<cabin::Cabin>(10, "A", "x"));
    u.addCabin(std::make_unique<cabin::Cabin>(20, "B", "y"));

    const auto cabins = u.getCabins();
    REQUIRE(cabins.size() == 2);
    CHECK(cabins[0]->getId() == 10);
    CHECK(cabins[1]->getId() == 20);
}

TEST_CASE("addCabin rejects a null cabin and a duplicate id")
{
    cabin::User u(1, "Kari");
    u.addCabin(std::make_unique<cabin::Cabin>(10, "A", "x"));

    CHECK_THROWS_AS(u.addCabin(nullptr), std::invalid_argument);
    CHECK_THROWS_AS(u.addCabin(std::make_unique<cabin::Cabin>(10, "B", "y")), std::invalid_argument);
    CHECK(u.getCabins().size() == 1);
}

TEST_CASE_FIXTURE(test::HardwareFixture, "removeCabin removes the cabin in any state; an unknown id does nothing")
{
    cabin::User u(1, "Kari");
    u.addCabin(std::make_unique<cabin::Cabin>(10, "A", "x"));
    u.addCabin(std::make_unique<cabin::Cabin>(20, "B", "y"));
    u.getCabins()[1]->addSensor(std::make_unique<cabin::SecuritySensor>(3, "Door"));   // Monitoring

    u.removeCabin(99);
    CHECK(u.getCabins().size() == 2);

    u.removeCabin(20);
    const auto cabins = u.getCabins();
    REQUIRE(cabins.size() == 1);
    CHECK(cabins[0]->getId() == 10);
}

TEST_CASE_FIXTURE(test::HardwareFixture, "alerts returned by checkSensors are delivered with addAlert")
{
    setSamples({{3, 1.0}});
    cabin::User u(1, "Kari");
    u.addCabin(std::make_unique<cabin::Cabin>(10, "A", "x"));
    cabin::Cabin& c = *u.getCabins().front();
    c.addSensor(std::make_unique<cabin::SecuritySensor>(3, "Door"));

    for (const cabin::Alert& a : c.checkSensors())
    {
        u.addAlert(a);
    }
    for (const cabin::Alert& a : c.checkSensors())
    {
        u.addAlert(a);
    }

    REQUIRE(u.getAlerts().size() == 2);
    CHECK(u.getAlerts()[0].getId() == 1);
    CHECK(u.getAlerts()[1].getId() == 2);
    CHECK(u.getAlerts()[1].getMessage() == "Door: intrusion (1)");
}
