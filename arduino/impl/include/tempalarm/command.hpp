#pragma once

#include "command_kind.hpp"

namespace tempalarm {

/// One parsed command. low and high are only meaningful for CommandKind::SetThreshold.
struct Command
{
    CommandKind kind;
    float low;
    float high;
};

}  // namespace tempalarm
