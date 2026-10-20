#pragma once

namespace tempalarm {

/// The temperature sensor on one pin (a DHT11 on the Arduino, see hal.hpp).
class TemperatureSensor
{
public:
    explicit TemperatureSensor(int pin);

    /// The temperature now in °C, or NaN when the sensor did not answer.
    float read();

private:
    int pin_;
};

}  // namespace tempalarm
