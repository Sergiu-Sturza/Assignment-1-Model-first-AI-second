#pragma once

#include <chrono>

namespace cabin {

/// The design's `DateTime`: a UTC point in time with whole-second precision.
/// A type alias, not a class, so it adds nothing to the class diagram.
using DateTime = std::chrono::sys_seconds;

}  // namespace cabin
