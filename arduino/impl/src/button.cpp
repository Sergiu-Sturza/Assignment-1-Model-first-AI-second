#include "tempalarm/button.hpp"

#include "tempalarm/hal.hpp"

namespace tempalarm {

namespace {

const uint32_t bounceMs = 50;

}  // namespace

Button::Button(int pin) : pin_(pin) {}

bool Button::wasPressed(uint32_t nowMs)
{
    const bool down = hal::isButtonDown(pin_);
    if (down == down_ || static_cast<uint32_t>(nowMs - changedMs_) < bounceMs)
    {
        return false;
    }
    down_ = down;
    changedMs_ = nowMs;
    return down;
}

}  // namespace tempalarm
