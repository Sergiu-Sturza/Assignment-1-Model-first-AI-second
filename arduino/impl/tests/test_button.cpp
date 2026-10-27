#include "board.hpp"
#include "doctest.h"
#include "tempalarm/button.hpp"

namespace sim = tempalarm::hal::sim;
using tempalarm::Button;

TEST_CASE_FIXTURE(test::FreshBoard, "Button: not pressed while up")
{
    Button button(test::upButtonPin);
    CHECK_FALSE(button.wasPressed(1000));
    CHECK_FALSE(button.wasPressed(2000));
}

TEST_CASE_FIXTURE(test::FreshBoard, "Button: one press counts once, at the moment it goes down")
{
    Button button(test::upButtonPin);
    sim::setButton(test::upButtonPin, true);
    CHECK(button.wasPressed(1000));
    CHECK_FALSE(button.wasPressed(1100));               // held: no second press
    CHECK_FALSE(button.wasPressed(5000));
    sim::setButton(test::upButtonPin, false);
    CHECK_FALSE(button.wasPressed(5100));               // released
    sim::setButton(test::upButtonPin, true);
    CHECK(button.wasPressed(5200));                     // pressed again
}

TEST_CASE_FIXTURE(test::FreshBoard, "Button: contact bounce within 50 ms is ignored")
{
    Button button(test::upButtonPin);
    sim::setButton(test::upButtonPin, true);
    CHECK(button.wasPressed(1000));
    sim::setButton(test::upButtonPin, false);           // bounces open...
    CHECK_FALSE(button.wasPressed(1010));
    sim::setButton(test::upButtonPin, true);            // ...and closed again
    CHECK_FALSE(button.wasPressed(1020));
    CHECK_FALSE(button.wasPressed(1100));               // still the same press
}

TEST_CASE_FIXTURE(test::FreshBoard, "Button: each button reads its own pin")
{
    Button up(test::upButtonPin);
    Button down(test::downButtonPin);
    sim::setButton(test::downButtonPin, true);
    CHECK_FALSE(up.wasPressed(1000));
    CHECK(down.wasPressed(1000));
}
