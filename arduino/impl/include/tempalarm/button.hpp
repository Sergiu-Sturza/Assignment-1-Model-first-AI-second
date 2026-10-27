#pragma once

#include <stdint.h>

namespace tempalarm {

/// A push button from one input pin to GND. Debounced: a change of the pin counts only
/// when the last accepted change is at least 50 ms old, so a press reacts at once and its
/// contact bounce is ignored.
class Button
{
public:
    explicit Button(int pin);

    /// Call as often as possible with the milliseconds since start.
    /// @return true once for each press, at the moment the button goes down
    bool wasPressed(uint32_t nowMs);

private:
    int pin_;
    bool down_ = false;
    uint32_t changedMs_ = 0;
};

}  // namespace tempalarm
