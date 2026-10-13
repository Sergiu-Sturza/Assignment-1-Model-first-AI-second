#pragma once

#include <string>

#include "cabin/sensor.hpp"

namespace cabin {

/// Detects movement or an opened door: a reading of 0 means quiet, anything else means
/// an intrusion, which alerts.
class SecuritySensor : public Sensor
{
public:
    SecuritySensor(int id, std::string name);

    Reading read() override;
    bool shouldAlert(const Reading& r) const override;
};

}  // namespace cabin
