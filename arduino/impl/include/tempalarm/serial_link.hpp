#pragma once

#include <stddef.h>

#include "text.hpp"

namespace tempalarm {

/// The line-oriented serial connection to the website (WebSerial on the other side).
/// Lines end with a newline; a carriage return before it is dropped. A line longer than
/// maxLineLength is discarded up to its newline, so it is never taken for a shorter one.
class SerialLink
{
public:
    SerialLink();

    /// Reads the characters that have arrived. Returns the line once its newline has
    /// arrived; nullptr while it is incomplete. The text stays valid until the next call.
    Text readLine();
    /// Sends @p text followed by a newline.
    void writeLine(Text text);

private:
    LineText line_;
    size_t length_ = 0;
    bool discarding_ = false;
};

}  // namespace tempalarm
