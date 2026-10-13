#include "cabin/alert.hpp"

#include <utility>

namespace cabin {

Alert::Alert(int id, DateTime time, Severity severity, std::string message)
    : id_(id), time_(time), severity_(severity), message_(std::move(message))
{
}

int Alert::getId() const { return id_; }

DateTime Alert::getTime() const { return time_; }

Severity Alert::getSeverity() const { return severity_; }

const std::string& Alert::getMessage() const { return message_; }

}  // namespace cabin
