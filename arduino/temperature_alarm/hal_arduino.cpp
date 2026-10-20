// The Arduino implementation of the hardware seam (src/tempalarm/hal.hpp): the only file
// that knows about the DHT11, the I2C LCD and the Serial port. The host build links
// impl/src/hal_host.cpp, a simulation, in its place.
#include <Arduino.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#include "board_config.h"
#include "src/tempalarm/hal.hpp"

namespace {

DHT dht(board::sensorPin, DHT11);
LiquidCrystal_I2C lcd(board::lcdAddress, board::lcdColumns, board::lcdRows);

}  // namespace

namespace board {

void begin()
{
    Serial.begin(baudRate);
    pinMode(buzzerPin, OUTPUT);
    digitalWrite(buzzerPin, LOW);
    dht.begin();
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Starting...");
}

}  // namespace board

namespace tempalarm {
namespace hal {

float readCelsius(int /*pin*/) { return dht.readTemperature(); }   // NaN when it fails

void writePin(int pin, bool high) { digitalWrite(pin, high ? HIGH : LOW); }

void lcdWrite(int row, Text text)
{
    lcd.setCursor(0, row);
    lcd.print(text);
}

bool serialAvailable() { return Serial.available() > 0; }

char serialRead() { return static_cast<char>(Serial.read()); }

void serialWriteLine(Text text) { Serial.println(text); }

}  // namespace hal
}  // namespace tempalarm
