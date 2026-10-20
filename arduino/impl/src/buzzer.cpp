#include "tempalarm/buzzer.hpp"

#include "tempalarm/hal.hpp"

namespace tempalarm {

Buzzer::Buzzer(int pin) : pin_(pin) {}

void Buzzer::turnOn()
{
    on_ = true;
    hal::writePin(pin_, true);
}

void Buzzer::turnOff()
{
    on_ = false;
    hal::writePin(pin_, false);
}

bool Buzzer::isOn() const { return on_; }

}  // namespace tempalarm
