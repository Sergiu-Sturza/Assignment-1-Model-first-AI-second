#pragma once

// How this board is wired, and the values it starts with. The only file to change for
// other pins, another LCD or another start threshold.

#include <stdint.h>

namespace board {

const uint8_t sensorPin = 2;      // DHT11 data
const uint8_t buzzerPin = 8;      // active or passive buzzer, + to the pin, - to GND
const unsigned int buzzerFrequencyHz = 2000;   // the alarm tone
// Push buttons, each from its pin to GND (the internal pull-up is used: pressed = LOW).
const uint8_t selectButtonPin = 5;   // switch between the low and the high limit
const uint8_t upButtonPin = 4;       // the selected limit + 0.1 °C
const uint8_t downButtonPin = 3;     // the selected limit - 0.1 °C
// LCD on I2C: Mega SDA = 20, SCL = 21 | Uno SDA = A4, SCL = A5
const uint8_t lcdAddress = 0x27;  // try 0x3F if the screen stays blank
const uint8_t lcdColumns = 16;
const uint8_t lcdRows = 2;

const unsigned long baudRate = 115200;     // the website connects at the same rate
const uint32_t measureIntervalMs = 2000;   // the DHT11 needs at least 1 s between reads

// The threshold at power-on, in °C; the website changes it with "SET,<low>,<high>".
const float startLow = 18.0f;
const float startHigh = 28.0f;

/// Starts the serial port, the sensor, the LCD, the buzzer and the button pins (in hal_arduino.cpp).
void begin();

}  // namespace board
