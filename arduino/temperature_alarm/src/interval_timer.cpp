#include "tempalarm/interval_timer.hpp"

namespace tempalarm {

IntervalTimer::IntervalTimer(uint32_t intervalMs) : intervalMs_(intervalMs) {}

bool IntervalTimer::isDue(uint32_t nowMs)
{
    // Unsigned subtraction: the elapsed time is right even after nowMs wrapped around.
    if (started_ && nowMs - lastMs_ < intervalMs_)
    {
        return false;
    }
    started_ = true;
    lastMs_ = nowMs;
    return true;
}

}  // namespace tempalarm
