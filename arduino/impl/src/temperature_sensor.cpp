#include "tempalarm/temperature_sensor.hpp"

#include "tempalarm/hal.hpp"

namespace tempalarm {

TemperatureSensor::TemperatureSensor(int pin) : pin_(pin) {}

float TemperatureSensor::read() { return hal::readCelsius(pin_); }

}  // namespace tempalarm
