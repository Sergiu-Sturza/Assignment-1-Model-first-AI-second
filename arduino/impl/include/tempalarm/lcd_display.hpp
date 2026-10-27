#pragma once

#include "temperature_status.hpp"
#include "threshold.hpp"
#include "threshold_limit.hpp"

namespace tempalarm {

/// A two-row character LCD: the temperature on the first row, its status on the second.
/// Every row is padded to the full width, so nothing of an older text stays visible.
class LcdDisplay
{
public:
    /// @param columns the display's width; at most 20 are used
    explicit LcdDisplay(int columns);

    /// "Temp: 23.4 °C" / "Status: OK", "ALARM: TOO HIGH" or "ALARM: TOO LOW".
    void showTemperature(float celsius, TemperatureStatus status);
    /// "Sensor error" / "Check wiring".
    void showSensorError();
    /// ">Low:  18.0 °C" / " High: 28.0 °C": both limits, the one the buttons change marked.
    void showThreshold(const Threshold& threshold, ThresholdLimit selected);

private:
    int columns_;
};

}  // namespace tempalarm
