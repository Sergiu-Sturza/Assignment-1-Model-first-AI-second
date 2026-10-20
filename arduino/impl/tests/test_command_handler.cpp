#include "board.hpp"
#include "doctest.h"
#include "tempalarm/command_handler.hpp"

namespace sim = tempalarm::hal::sim;

namespace {

struct Rig : test::FreshBoard
{
    tempalarm::Threshold threshold{18.0f, 28.0f};
    tempalarm::SerialLink link;
    tempalarm::CommandHandler commands{link, threshold};
};

}  // namespace

TEST_CASE_FIXTURE(Rig, "CommandHandler: nothing received, nothing done")
{
    commands.poll();
    CHECK(sim::sent().empty());
}

TEST_CASE_FIXTURE(Rig, "CommandHandler: SET changes the threshold and confirms it")
{
    sim::receive("SET,20.0,30.0\n");
    commands.poll();
    CHECK(threshold.getLow() == 20.0f);
    CHECK(threshold.getHigh() == 30.0f);
    REQUIRE(sim::sent().size() == 1);
    CHECK(sim::sent()[0] == "TH,20.0,30.0");
}

TEST_CASE_FIXTURE(Rig, "CommandHandler: an invalid range is rejected, the threshold kept")
{
    sim::receive("SET,30,20\n");
    commands.poll();
    CHECK(threshold.getLow() == 18.0f);
    CHECK(threshold.getHigh() == 28.0f);
    CHECK(sim::sent().back() == "ERR,INVALID");
}

TEST_CASE_FIXTURE(Rig, "CommandHandler: GET answers with the threshold")
{
    sim::receive("GET\n");
    commands.poll();
    CHECK(sim::sent().back() == "TH,18.0,28.0");
}

TEST_CASE_FIXTURE(Rig, "CommandHandler: an unknown line is answered with an error")
{
    sim::receive("HELLO\n");
    commands.poll();
    CHECK(sim::sent().back() == "ERR,INVALID");
}

TEST_CASE_FIXTURE(Rig, "CommandHandler: one line per poll, in order")
{
    sim::receive("SET,10,20\nGET\n");
    commands.poll();
    CHECK(sim::sent().size() == 1);
    commands.poll();
    REQUIRE(sim::sent().size() == 2);
    CHECK(sim::sent()[1] == "TH,10.0,20.0");
}
