#include "tempalarm/command_handler.hpp"

namespace tempalarm {

CommandHandler::CommandHandler(SerialLink& link, Threshold& threshold)
    : link_(link), threshold_(threshold)
{
}

void CommandHandler::poll()
{
    const Text line = link_.readLine();
    if (line == nullptr)
    {
        return;
    }
    const Command command = parser_.parse(line);
    Text reply = nullptr;
    if (command.kind == CommandKind::SetThreshold && threshold_.set(command.low, command.high))
    {
        reply = formatter_.formatThreshold(threshold_);
    }
    else if (command.kind == CommandKind::GetThreshold)
    {
        reply = formatter_.formatThreshold(threshold_);
    }
    else
    {
        reply = formatter_.formatRejected();   // unknown, malformed, or an invalid range
    }
    link_.writeLine(reply);
}

}  // namespace tempalarm
