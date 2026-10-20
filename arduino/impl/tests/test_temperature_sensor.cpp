#include <cmath>

#include "board.hpp"
#include "doctest.h"
#include "tempalarm/temperature_sensor.hpp"

namespace sim = tempalarm::hal::sim;
using tempalarm::TemperatureSensor;

TEST_CASE_FIXTURE(test::FreshBoard, "TemperatureSensor: reads the board, NaN when it fails")
{
    TemperatureSensor sensor(test::sensorPin);
    sim::setCelsius(24.5f);
    CHECK(sensor.read() == 24.5f);
    sim::setCelsius(NAN);
    CHECK(std::isnan(sensor.read()));
}
