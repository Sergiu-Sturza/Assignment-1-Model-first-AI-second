#include "tempalarm/lcd_display.hpp"

#include "number_text.hpp"
#include "tempalarm/hal.hpp"

namespace tempalarm {

namespace {

const int widest = 20;

// The degree sign in the character ROM of the HD44780 the I2C LCD modules use.
const char degreeSign[] = "\xDF";

/// Writes @p text on @p row, padded with spaces to @p columns.
void writeRow(int row, const char* text, int columns)
{
    char padded[widest + 1];
    size_t length = detail::append(padded, columns + 1, 0, text);
    while (length < static_cast<size_t>(columns))
    {
        padded[length++] = ' ';
    }
    padded[length] = '\0';
    hal::lcdWrite(row, padded);
}

const char* statusText(TemperatureStatus status)
{
    switch (status)
    {
    case TemperatureStatus::Ok: return "Status: OK";
    case TemperatureStatus::TooLow: return "ALARM: TOO LOW";
    case TemperatureStatus::TooHigh: return "ALARM: TOO HIGH";
    case TemperatureStatus::SensorError: break;
    }
    return "Status: ERROR";
}

}  // namespace

LcdDisplay::LcdDisplay(int columns)
    : columns_(columns < 1 ? 1 : (columns > widest ? widest : columns))
{
}

void LcdDisplay::showTemperature(float celsius, TemperatureStatus status)
{
    char line[widest + 1];
    size_t length = detail::append(line, sizeof line, 0, "Temp: ");
    length = detail::appendDecimal(line, sizeof line, length, celsius);
    length = detail::append(line, sizeof line, length, " ");
    length = detail::append(line, sizeof line, length, degreeSign);
    detail::append(line, sizeof line, length, "C");
    writeRow(0, line, columns_);
    writeRow(1, statusText(status), columns_);
}

void LcdDisplay::showSensorError()
{
    writeRow(0, "Sensor error", columns_);
    writeRow(1, "Check wiring", columns_);
}

}  // namespace tempalarm
