#pragma once

#include "command_parser.hpp"
#include "message_formatter.hpp"
#include "serial_link.hpp"
#include "threshold.hpp"

namespace tempalarm {

/// Carries out the commands the website sends: SET changes the threshold, GET asks for
/// it. Every command is answered, with the threshold ("TH,...") or with "ERR,INVALID".
class CommandHandler
{
public:
    CommandHandler(SerialLink& link, Threshold& threshold);

    CommandHandler(const CommandHandler&) = delete;
    CommandHandler& operator=(const CommandHandler&) = delete;

    /// Call as often as possible: handles at most one complete received line per call.
    void poll();

private:
    SerialLink& link_;              // CommandHandler --> SerialLink : link
    Threshold& threshold_;          // CommandHandler --> Threshold : threshold
    CommandParser parser_;          // CommandHandler *-- CommandParser : parser
    MessageFormatter formatter_;    // CommandHandler *-- MessageFormatter : formatter
};

}  // namespace tempalarm
