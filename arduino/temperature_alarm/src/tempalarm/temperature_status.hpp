#pragma once

namespace tempalarm {

/// What one measurement means, compared with the threshold (class diagram: «enumeration»).
enum class TemperatureStatus { Ok, TooLow, TooHigh, SensorError };

}  // namespace tempalarm
