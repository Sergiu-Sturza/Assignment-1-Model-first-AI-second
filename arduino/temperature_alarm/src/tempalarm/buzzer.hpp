#pragma once

namespace tempalarm {

/// An active buzzer on one output pin: it sounds while the pin is high.
class Buzzer
{
public:
    explicit Buzzer(int pin);

    void turnOn();
    void turnOff();
    bool isOn() const;

private:
    int pin_;
    bool on_ = false;
};

}  // namespace tempalarm
