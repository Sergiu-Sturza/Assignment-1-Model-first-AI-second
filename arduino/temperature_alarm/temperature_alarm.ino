// Temperature alarm: reads a DHT11, sounds a buzzer outside the threshold, shows the
// temperature on an I2C LCD and talks to the website (../dashboard) over WebSerial. Three
// buttons change the threshold on the board itself. Built for an Arduino Mega.
//
// This file only creates the objects of the design and runs them; it holds no behaviour.
//   behaviour  src/        the classes of ../impl, verified against ../diagrams
//                          (copied here by ../tools/sync_firmware.py; edit ../impl instead)
//   hardware   hal_arduino.cpp
//   wiring     board_config.h
//
// Libraries: "DHT sensor library" (Adafruit) and "LiquidCrystal I2C" (Frank de Brabander).

#include "board_config.h"
#include "src/tempalarm/command_handler.hpp"
#include "src/tempalarm/temperature_monitor.hpp"
#include "src/tempalarm/threshold_editor.hpp"

using namespace tempalarm;

Threshold threshold(board::startLow, board::startHigh);
TemperatureSensor sensor(board::sensorPin);
Buzzer buzzer(board::buzzerPin);
LcdDisplay display(board::lcdColumns);
SerialLink serialLink;
TemperatureMonitor monitor(sensor, threshold, buzzer, display, serialLink, board::measureIntervalMs);
CommandHandler commands(serialLink, threshold);
ThresholdEditor editor(board::selectButtonPin, board::upButtonPin, board::downButtonPin,
                       threshold, display, serialLink);

void setup()
{
    board::begin();
}

void loop()
{
    commands.poll();               // a threshold change from the website, if one arrived
    editor.poll(millis());         // a threshold change from the buttons, if one was pressed
    monitor.update(millis());      // a measurement, if one is due
}
