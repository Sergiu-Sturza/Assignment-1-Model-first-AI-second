#pragma once

#include <stdint.h>

namespace tempalarm {

/// Says when the next periodic measurement is due. Due at the first call, then every
/// intervalMs; correct across the wrap-around of the millisecond counter (49.7 days).
class IntervalTimer
{
public:
    explicit IntervalTimer(uint32_t intervalMs);

    /// True once per interval; restarts the interval when it returns true.
    bool isDue(uint32_t nowMs);

private:
    uint32_t intervalMs_;
    uint32_t lastMs_ = 0;
    bool started_ = false;
};

}  // namespace tempalarm
