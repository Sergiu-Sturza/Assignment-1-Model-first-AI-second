#pragma once

#include <string>

#include "cabin/date_time.hpp"
#include "cabin/severity.hpp"

namespace cabin {

/// A warning raised for one sensor reading that is outside its limits.
///
/// Created by Cabin::checkSensors() and handed to a User with User::addAlert().
class Alert
{
public:
    /// @param id       unique within the cabin that raised it, 1, 2, 3, ...
    /// @param time     the time of the reading that raised it
    /// @param severity how urgent it is
    /// @param message  "<sensor name>: <kind> (<value><unit>)"
    Alert(int id, DateTime time, Severity severity, std::string message);

    int getId() const;
    DateTime getTime() const;
    Severity getSeverity() const;
    const std::string& getMessage() const;

private:
    int id_;
    DateTime time_;
    Severity severity_;
    std::string message_;
};

}  // namespace cabin
