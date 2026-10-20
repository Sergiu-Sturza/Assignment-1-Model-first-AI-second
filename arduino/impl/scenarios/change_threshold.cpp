// The scenario of diagrams/input/sequence-change_threshold.drawio.
//
// main() sets the objects up; scenario() makes the calls the diagram shows, and only those
// are recorded. The verifier runs this program in a traced build.
#include "tempalarm/command_handler.hpp"
#include "tempalarm/hal_sim.hpp"

namespace {

void scenario(tempalarm::CommandHandler& commands)
{
    commands.poll();   // Loop -> CommandHandler : poll()
}

}  // namespace

int main()
{
    // The line the website sends when the user saves 20.0 - 30.0 °C.
    tempalarm::hal::sim::receive("SET,20.0,30.0\n");

    tempalarm::Threshold threshold(18.0f, 28.0f);
    tempalarm::SerialLink link;
    tempalarm::CommandHandler commands(link, threshold);
    scenario(commands);
    return 0;
}
