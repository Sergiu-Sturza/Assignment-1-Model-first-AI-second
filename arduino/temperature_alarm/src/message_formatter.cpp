#include "tempalarm/message_formatter.hpp"

#include "number_text.hpp"

namespace tempalarm {

namespace {

const char* statusCode(TemperatureStatus status)
{
    switch (status)
    {
    case TemperatureStatus::Ok: return "OK";
    case TemperatureStatus::TooLow: return "LOW";
    case TemperatureStatus::TooHigh: return "HIGH";
    case TemperatureStatus::SensorError: break;
    }
    return "ERROR";
}

}  // namespace

MessageFormatter::MessageFormatter() { text_[0] = '\0'; }

Text MessageFormatter::formatReading(float celsius, TemperatureStatus status,
                                     const Threshold& threshold)
{
    const size_t size = sizeof text_;
    size_t length = detail::append(text_, size, 0, "T,");
    if (status != TemperatureStatus::SensorError)
    {
        length = detail::appendDecimal(text_, size, length, celsius);
    }
    length = detail::append(text_, size, length, ",");
    length = detail::append(text_, size, length, statusCode(status));
    length = detail::append(text_, size, length, ",");
    length = detail::appendDecimal(text_, size, length, threshold.getLow());
    length = detail::append(text_, size, length, ",");
    detail::appendDecimal(text_, size, length, threshold.getHigh());
    return text_;
}

Text MessageFormatter::formatThreshold(const Threshold& threshold)
{
    const size_t size = sizeof text_;
    size_t length = detail::append(text_, size, 0, "TH,");
    length = detail::appendDecimal(text_, size, length, threshold.getLow());
    length = detail::append(text_, size, length, ",");
    detail::appendDecimal(text_, size, length, threshold.getHigh());
    return text_;
}

Text MessageFormatter::formatRejected()
{
    detail::append(text_, sizeof text_, 0, "ERR,INVALID");
    return text_;
}

}  // namespace tempalarm
