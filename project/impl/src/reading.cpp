#include "cabin/reading.hpp"

namespace cabin {

Reading::Reading(DateTime time, double value) : time_(time), value_(value) {}

DateTime Reading::getTime() const { return time_; }

double Reading::getValue() const { return value_; }

}  // namespace cabin
