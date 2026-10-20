#include "tempalarm/serial_link.hpp"

#include "tempalarm/hal.hpp"

namespace tempalarm {

SerialLink::SerialLink() { line_[0] = '\0'; }

Text SerialLink::readLine()
{
    while (hal::serialAvailable())
    {
        const char c = hal::serialRead();
        if (c == '\n')
        {
            const bool complete = !discarding_ && length_ > 0;   // empty lines are ignored
            line_[length_] = '\0';
            length_ = 0;
            discarding_ = false;
            if (complete)
            {
                return line_;
            }
            continue;
        }
        if (c == '\r' || discarding_)
        {
            continue;
        }
        if (length_ == maxLineLength)                           // too long: drop it all
        {
            discarding_ = true;
            length_ = 0;
            continue;
        }
        line_[length_++] = c;
    }
    return nullptr;
}

void SerialLink::writeLine(Text text) { hal::serialWriteLine(text); }

}  // namespace tempalarm
