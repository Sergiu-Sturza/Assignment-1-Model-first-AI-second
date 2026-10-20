#pragma once

#include "temperature_status.hpp"
#include "text.hpp"
#include "threshold.hpp"

namespace tempalarm {

/// Writes the lines the device sends to the website (read by dashboard/js/protocol.js):
///
///     T,<celsius>,<OK|LOW|HIGH|ERROR>,<low>,<high>    one measurement; no celsius on ERROR
///     TH,<low>,<high>                                  the threshold now
///     ERR,INVALID                                      a command that was not accepted
///
/// Numbers have one decimal. The returned text stays valid until the next call.
class MessageFormatter
{
public:
    MessageFormatter();

    Text formatReading(float celsius, TemperatureStatus status, const Threshold& threshold);
    Text formatThreshold(const Threshold& threshold);
    Text formatRejected();

private:
    MessageText text_;
};

}  // namespace tempalarm
