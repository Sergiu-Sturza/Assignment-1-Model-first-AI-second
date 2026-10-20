#pragma once

// What every test starts from: the simulated board reset, and the wiring of the sketch.

#include "tempalarm/hal_sim.hpp"

namespace test {

const int sensorPin = 2;
const int buzzerPin = 8;

/// Resets the simulated board when a test begins, so no test sees another's state.
struct FreshBoard
{
    FreshBoard() { tempalarm::hal::sim::reset(); }
};

}  // namespace test
