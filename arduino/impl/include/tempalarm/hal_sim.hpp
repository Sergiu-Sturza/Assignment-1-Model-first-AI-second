#pragma once

// Host only (src/hal_host.cpp): the simulated board behind hal.hpp. Tests, the benchmark,
// the demo and the scenarios set the temperature and the received characters, and read
// back the pins, the LCD rows and what was sent. Never copied to the Arduino sketch.

#include <string>
#include <vector>

namespace tempalarm {
namespace hal {
namespace sim {

/// What readCelsius() returns from now on, for every pin. NaN simulates a broken sensor.
void setCelsius(float celsius);
/// Characters that "arrive" on the serial line, read one by one by serialRead().
void receive(const std::string& characters);
/// The level last written to @p pin; false if never written.
bool pin(int pin);
/// The text last written to LCD row @p row.
std::string lcdRow(int row);
/// Every line sent with serialWriteLine(), oldest first.
const std::vector<std::string>& sent();
/// Whether sent lines are kept for sent() (default true); the benchmark turns it off so
/// that it times the classes, not the growth of this log.
void keepSent(bool keep);
/// Back to the start: 20 °C, nothing received or sent, every pin low, the LCD blank.
void reset();

}  // namespace sim
}  // namespace hal
}  // namespace tempalarm
