#pragma once

// The hardware seam: the few things the classes need from the board. Free functions, not
// classes, so they are neither part of the class diagram nor messages in a sequence
// diagram. Linked, not injected: the Arduino sketch implements them in hal_arduino.cpp
// (DHT11, I2C LCD, Serial), the host build in src/hal_host.cpp (a simulation that tests,
// the benchmark and the scenarios control through hal_sim.hpp).

#include "text.hpp"

namespace tempalarm {
namespace hal {

/// The temperature at the sensor on @p pin in °C, or NaN when it does not answer.
float readCelsius(int pin);
/// Drives output @p pin high or low.
void writePin(int pin, bool high);
/// Shows @p text on LCD row @p row, from the first column.
void lcdWrite(int row, Text text);
/// Whether a received serial character is waiting.
bool serialAvailable();
/// The next received character; only call when serialAvailable().
char serialRead();
/// Sends @p text and a newline.
void serialWriteLine(Text text);

}  // namespace hal
}  // namespace tempalarm
