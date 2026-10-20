#include <string>

#include "board.hpp"
#include "doctest.h"
#include "tempalarm/serial_link.hpp"

namespace sim = tempalarm::hal::sim;
using tempalarm::SerialLink;

TEST_CASE_FIXTURE(test::FreshBoard, "SerialLink: nothing until the newline arrives")
{
    SerialLink link;
    CHECK(link.readLine() == nullptr);
    sim::receive("SET,2");
    CHECK(link.readLine() == nullptr);
    sim::receive("0,30\n");
    const char* line = link.readLine();
    REQUIRE(line != nullptr);
    CHECK(std::string(line) == "SET,20,30");
}

TEST_CASE_FIXTURE(test::FreshBoard, "SerialLink: one line per call, CR LF accepted")
{
    SerialLink link;
    sim::receive("GET\r\nSET,1,2\n");
    CHECK(std::string(link.readLine()) == "GET");
    CHECK(std::string(link.readLine()) == "SET,1,2");
    CHECK(link.readLine() == nullptr);
}

TEST_CASE_FIXTURE(test::FreshBoard, "SerialLink: empty lines are skipped")
{
    SerialLink link;
    sim::receive("\n\r\nGET\n");
    CHECK(std::string(link.readLine()) == "GET");
}

TEST_CASE_FIXTURE(test::FreshBoard, "SerialLink: a too long line is dropped up to its newline")
{
    SerialLink link;
    sim::receive(std::string(tempalarm::maxLineLength, 'x') + "\n");   // just fits
    CHECK(std::string(link.readLine()).size() == tempalarm::maxLineLength);
    sim::receive(std::string(tempalarm::maxLineLength + 1, 'x') + "SET,1,2\nGET\n");
    CHECK(std::string(link.readLine()) == "GET");                      // not "SET,1,2"
}

TEST_CASE_FIXTURE(test::FreshBoard, "SerialLink: writeLine sends one line")
{
    SerialLink link;
    link.writeLine("TH,18.0,28.0");
    REQUIRE(sim::sent().size() == 1);
    CHECK(sim::sent()[0] == "TH,18.0,28.0");
}
