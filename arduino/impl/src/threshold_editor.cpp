#include "tempalarm/threshold_editor.hpp"

namespace tempalarm {

namespace {

/// @p celsius moved by @p steps tenths of a degree, on the 0.1 °C grid, so that repeated
/// steps do not collect float rounding errors.
float stepped(float celsius, int steps)
{
    const long tenths = static_cast<long>(celsius * 10.0f + (celsius < 0.0f ? -0.5f : 0.5f));
    return static_cast<float>(tenths + steps) / 10.0f;
}

}  // namespace

ThresholdEditor::ThresholdEditor(int selectPin, int upPin, int downPin, Threshold& threshold,
                                 LcdDisplay& display, SerialLink& link)
    : selectButton_(selectPin), upButton_(upPin), downButton_(downPin), threshold_(threshold),
      display_(display), link_(link)
{
}

void ThresholdEditor::poll(uint32_t nowMs)
{
    // Every button is asked on every call, so each one keeps its debouncing up to date.
    const bool select = selectButton_.wasPressed(nowMs);
    const bool up = upButton_.wasPressed(nowMs);
    const bool down = downButton_.wasPressed(nowMs);
    if (select)
    {
        selected_ = selected_ == ThresholdLimit::Low ? ThresholdLimit::High : ThresholdLimit::Low;
    }
    const int steps = (up ? 1 : 0) - (down ? 1 : 0);
    bool changed = false;
    if (steps != 0)
    {
        float low = threshold_.getLow();
        float high = threshold_.getHigh();
        if (selected_ == ThresholdLimit::Low)
        {
            low = stepped(low, steps);
        }
        else
        {
            high = stepped(high, steps);
        }
        changed = threshold_.set(low, high);
    }
    if (select || up || down)
    {
        display_.showThreshold(threshold_, selected_);
    }
    if (changed)
    {
        link_.writeLine(formatter_.formatThreshold(threshold_));
    }
}

}  // namespace tempalarm
