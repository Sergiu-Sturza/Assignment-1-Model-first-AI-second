#pragma once

#include "command.hpp"
#include "text.hpp"

namespace tempalarm {

/// Reads the lines the website sends: "SET,<low>,<high>" and "GET". Anything else, or a
/// SET whose numbers do not parse, is CommandKind::Invalid. Whether the numbers make a
/// valid range is for the Threshold to decide, not the parser.
class CommandParser
{
public:
    Command parse(Text line) const;
};

}  // namespace tempalarm
