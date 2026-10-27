#include <cstdint>

#include "board.hpp"
#include "doctest.h"
#include "tempalarm/threshold_editor.hpp"

namespace sim = tempalarm::hal::sim;

namespace {

/// The sketch's editor on the simulated board, with the start threshold 18.0 - 28.0 °C.
struct Rig : test::FreshBoard
{
    tempalarm::Threshold threshold{18.0f, 28.0f};
    tempalarm::LcdDisplay display{16};
    tempalarm::SerialLink link;
    tempalarm::ThresholdEditor editor{test::selectButtonPin, test::upButtonPin,
                                      test::downButtonPin, threshold, display, link};
    std::uint32_t now = 1000;

    /// Presses the button on @p pin and lets it go, 100 ms each.
    void press(int pin)
    {
        sim::setButton(pin, true);
        editor.poll(now += 100);
        sim::setButton(pin, false);
        editor.poll(now += 100);
    }
};

}  // namespace

TEST_CASE_FIXTURE(Rig, "ThresholdEditor: nothing happens without a press")
{
    editor.poll(now);
    CHECK(sim::lcdRow(0).empty());
    CHECK(sim::sent().empty());
}

TEST_CASE_FIXTURE(Rig, "ThresholdEditor: up and down change the low limit by 0.1 at start")
{
    press(test::upButtonPin);
    CHECK(threshold.getLow() == doctest::Approx(18.1f));
    CHECK(threshold.getHigh() == doctest::Approx(28.0f));
    CHECK(sim::sent().back() == "TH,18.1,28.0");
    CHECK(sim::lcdRow(0) == ">Low:  18.1 \xDF" "C  ");
    press(test::downButtonPin);
    press(test::downButtonPin);
    CHECK(threshold.getLow() == doctest::Approx(17.9f));
    CHECK(sim::sent().back() == "TH,17.9,28.0");
}

TEST_CASE_FIXTURE(Rig, "ThresholdEditor: select switches to the high limit and back")
{
    press(test::selectButtonPin);
    CHECK(sim::lcdRow(1) == ">High: 28.0 \xDF" "C  ");
    CHECK(sim::sent().empty());                         // selecting changes nothing
    press(test::upButtonPin);
    CHECK(threshold.getLow() == doctest::Approx(18.0f));
    CHECK(threshold.getHigh() == doctest::Approx(28.1f));
    press(test::selectButtonPin);
    press(test::downButtonPin);
    CHECK(threshold.getLow() == doctest::Approx(17.9f));
}

TEST_CASE_FIXTURE(Rig, "ThresholdEditor: many steps stay on the 0.1 grid")
{
    for (int i = 0; i < 25; ++i)
    {
        press(test::upButtonPin);
    }
    CHECK(sim::sent().back() == "TH,20.5,28.0");
    CHECK(threshold.getLow() == doctest::Approx(20.5f).epsilon(1e-6));
}

TEST_CASE_FIXTURE(Rig, "ThresholdEditor: a step that makes an invalid range is not made")
{
    REQUIRE(threshold.set(27.9f, 28.0f));
    press(test::upButtonPin);                           // low would equal high
    CHECK(threshold.getLow() == doctest::Approx(27.9f));
    CHECK(sim::sent().empty());
    CHECK(sim::lcdRow(0) == ">Low:  27.9 \xDF" "C  ");  // shown, unchanged
}
