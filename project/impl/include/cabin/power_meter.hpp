#pragma once

#include <string>

#include "cabin/sensor.hpp"

namespace cabin {

/// Measures the energy used in one measurement interval, in kWh. Alerts when a reading is
/// above the threshold; a value exactly at the threshold does not alert.
class PowerMeter : public Sensor
{
public:
    PowerMeter(int id, std::string name, double threshold = 3.0);

    Reading read() override;
    bool shouldAlert(const Reading& r) const override;

    /// Sets the highest allowed consumption per interval, in kWh.
    void setThreshold(double t);

private:
    double threshold_;
};

}  // namespace cabin
