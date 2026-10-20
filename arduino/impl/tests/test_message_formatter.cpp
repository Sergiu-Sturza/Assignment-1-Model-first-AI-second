#include <cmath>
#include <cstring>
#include <string>

#include "doctest.h"
#include "tempalarm/message_formatter.hpp"

using tempalarm::MessageFormatter;
using tempalarm::TemperatureStatus;
using tempalarm::Threshold;

TEST_CASE("MessageFormatter: a reading, one decimal, with the threshold")
{
    MessageFormatter f;
    const Threshold t(18.0f, 28.0f);
    CHECK(std::string(f.formatReading(23.44f, TemperatureStatus::Ok, t)) == "T,23.4,OK,18.0,28.0");
    CHECK(std::string(f.formatReading(31.55f, TemperatureStatus::TooHigh, t)) == "T,31.6,HIGH,18.0,28.0");
    CHECK(std::string(f.formatReading(-3.25f, TemperatureStatus::TooLow, t)) == "T,-3.3,LOW,18.0,28.0");
    CHECK(std::string(f.formatReading(0.0f, TemperatureStatus::TooLow, t)) == "T,0.0,LOW,18.0,28.0");
}

TEST_CASE("MessageFormatter: a failed reading has no temperature")
{
    MessageFormatter f;
    CHECK(std::string(f.formatReading(NAN, TemperatureStatus::SensorError, Threshold(5.0f, 9.5f)))
          == "T,,ERROR,5.0,9.5");
}

TEST_CASE("MessageFormatter: threshold and rejection")
{
    MessageFormatter f;
    CHECK(std::string(f.formatThreshold(Threshold(-12.5f, 40.0f))) == "TH,-12.5,40.0");
    CHECK(std::string(f.formatRejected()) == "ERR,INVALID");
}

TEST_CASE("MessageFormatter: the longest message fits its buffer")
{
    MessageFormatter f;
    const char* text = f.formatReading(-99999.0f, TemperatureStatus::TooHigh, Threshold(-99999.0f, -99999.0f));
    CHECK(std::string(text) == "T,-99999.0,HIGH,-99999.0,-99999.0");
    CHECK(std::strlen(text) <= tempalarm::maxMessageLength);
}
