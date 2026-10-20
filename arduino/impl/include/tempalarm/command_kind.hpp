#pragma once

namespace tempalarm {

/// What a line received from the website asks for (class diagram: «enumeration»).
enum class CommandKind { SetThreshold, GetThreshold, Invalid };

}  // namespace tempalarm
