#pragma once

#include <stdint.h>

#include "button.hpp"
#include "lcd_display.hpp"
#include "message_formatter.hpp"
#include "serial_link.hpp"
#include "threshold.hpp"
#include "threshold_limit.hpp"

namespace tempalarm {

/// Carries out the three buttons on the board: one selects the low or the high limit, one
/// raises it by 0.1 °C, one lowers it by 0.1 °C. Every press shows the threshold on the
/// LCD; every change is also sent to the website ("TH,<low>,<high>"). A change that would
/// make an invalid range (see Threshold::set) is not made.
class ThresholdEditor
{
public:
    ThresholdEditor(int selectPin, int upPin, int downPin, Threshold& threshold,
                    LcdDisplay& display, SerialLink& link);

    ThresholdEditor(const ThresholdEditor&) = delete;
    ThresholdEditor& operator=(const ThresholdEditor&) = delete;

    /// Call as often as possible with the milliseconds since start.
    void poll(uint32_t nowMs);

private:
    ThresholdLimit selected_ = ThresholdLimit::Low;

    Button selectButton_;           // ThresholdEditor *-- Button : selectButton
    Button upButton_;               // ThresholdEditor *-- Button : upButton
    Button downButton_;             // ThresholdEditor *-- Button : downButton
    Threshold& threshold_;          // ThresholdEditor --> Threshold : threshold
    LcdDisplay& display_;           // ThresholdEditor --> LcdDisplay : display
    SerialLink& link_;              // ThresholdEditor --> SerialLink : link
    MessageFormatter formatter_;    // ThresholdEditor *-- MessageFormatter : formatter
};

}  // namespace tempalarm
