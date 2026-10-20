#include "board.hpp"
#include "doctest.h"
#include "tempalarm/buzzer.hpp"

namespace sim = tempalarm::hal::sim;
using tempalarm::Buzzer;

TEST_CASE_FIXTURE(test::FreshBoard, "Buzzer: off at start, the pin follows turnOn and turnOff")
{
    Buzzer buzzer(test::buzzerPin);
    CHECK_FALSE(buzzer.isOn());
    buzzer.turnOn();
    CHECK(buzzer.isOn());
    CHECK(sim::pin(test::buzzerPin));
    buzzer.turnOff();
    CHECK_FALSE(buzzer.isOn());
    CHECK_FALSE(sim::pin(test::buzzerPin));
}
