#include <string>

#include "board.hpp"
#include "doctest.h"
#include "tempalarm/lcd_display.hpp"

namespace sim = tempalarm::hal::sim;
using tempalarm::LcdDisplay;
using tempalarm::TemperatureStatus;

TEST_CASE_FIXTURE(test::FreshBoard, "LcdDisplay: temperature and status, padded to the width")
{
    LcdDisplay lcd(16);
    lcd.showTemperature(23.44f, TemperatureStatus::Ok);
    CHECK(sim::lcdRow(0) == "Temp: 23.4 \xDF" "C   ");
    CHECK(sim::lcdRow(1) == "Status: OK      ");
}

TEST_CASE_FIXTURE(test::FreshBoard, "LcdDisplay: the alarm texts")
{
    LcdDisplay lcd(16);
    lcd.showTemperature(30.0f, TemperatureStatus::TooHigh);
    CHECK(sim::lcdRow(1) == "ALARM: TOO HIGH ");
    lcd.showTemperature(-2.0f, TemperatureStatus::TooLow);
    CHECK(sim::lcdRow(0) == "Temp: -2.0 \xDF" "C   ");
    CHECK(sim::lcdRow(1) == "ALARM: TOO LOW  ");
}

TEST_CASE_FIXTURE(test::FreshBoard, "LcdDisplay: sensor error")
{
    LcdDisplay lcd(16);
    lcd.showSensorError();
    CHECK(sim::lcdRow(0) == "Sensor error    ");
    CHECK(sim::lcdRow(1) == "Check wiring    ");
}

TEST_CASE_FIXTURE(test::FreshBoard, "LcdDisplay: text is cut at a narrow display, at most 20 columns used")
{
    LcdDisplay narrow(8);
    narrow.showTemperature(21.0f, TemperatureStatus::TooHigh);
    CHECK(sim::lcdRow(1) == "ALARM: T");
    LcdDisplay wide(40);
    wide.showSensorError();
    CHECK(sim::lcdRow(0).size() == 20);
}

TEST_CASE_FIXTURE(test::FreshBoard, "LcdDisplay: the threshold, the selected limit marked")
{
    LcdDisplay lcd(16);
    const tempalarm::Threshold threshold(18.0f, 28.5f);
    lcd.showThreshold(threshold, tempalarm::ThresholdLimit::Low);
    CHECK(sim::lcdRow(0) == ">Low:  18.0 \xDF" "C  ");
    CHECK(sim::lcdRow(1) == " High: 28.5 \xDF" "C  ");
    lcd.showThreshold(threshold, tempalarm::ThresholdLimit::High);
    CHECK(sim::lcdRow(0) == " Low:  18.0 \xDF" "C  ");
    CHECK(sim::lcdRow(1) == ">High: 28.5 \xDF" "C  ");
}
