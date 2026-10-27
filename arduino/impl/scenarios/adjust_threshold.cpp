// The scenario of diagrams/input/sequence-adjust_threshold.drawio.
//
// main() sets the objects up; scenario() makes the calls the diagram shows, and only those
// are recorded. The verifier runs this program in a traced build.
#include "tempalarm/hal_sim.hpp"
#include "tempalarm/threshold_editor.hpp"

namespace {

void scenario(tempalarm::ThresholdEditor& editor)
{
    editor.poll(1000);   // Loop -> ThresholdEditor : poll(nowMs)
}

}  // namespace

int main()
{
    // The user holds the up button (D4) down; the low limit is selected at start.
    tempalarm::hal::sim::setButton(4, true);

    tempalarm::Threshold threshold(18.0f, 28.0f);
    tempalarm::LcdDisplay display(16);
    tempalarm::SerialLink link;
    tempalarm::ThresholdEditor editor(5, 4, 3, threshold, display, link);
    scenario(editor);
    return 0;
}
