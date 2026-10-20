#include <initializer_list>

#include "doctest.h"
#include "tempalarm/command_parser.hpp"

using tempalarm::CommandKind;
using tempalarm::CommandParser;

TEST_CASE("CommandParser: SET with two numbers")
{
    const auto c = CommandParser().parse("SET,20.5,30");
    CHECK(c.kind == CommandKind::SetThreshold);
    CHECK(c.low == 20.5f);
    CHECK(c.high == 30.0f);
}

TEST_CASE("CommandParser: negative numbers")
{
    const auto c = CommandParser().parse("SET,-10.0,-2.5");
    CHECK(c.kind == CommandKind::SetThreshold);
    CHECK(c.low == -10.0f);
    CHECK(c.high == -2.5f);
}

TEST_CASE("CommandParser: the range is not judged here")
{
    CHECK(CommandParser().parse("SET,30,20").kind == CommandKind::SetThreshold);
}

TEST_CASE("CommandParser: GET")
{
    CHECK(CommandParser().parse("GET").kind == CommandKind::GetThreshold);
}

TEST_CASE("CommandParser: anything else is invalid")
{
    const CommandParser p;
    for (const char* line : {"", "get", "GET,1", "SET", "SET,", "SET,20", "SET,20,", "SET,,30",
                             "SET,a,30", "SET,20,b", "SET,20,30,40", "SET,20,30x", "SET 20 30",
                             "HELLO",
                             // only the syntax protocol.js writes: no spaces, exponents,
                             // signs other than a leading minus, or special values
                             "SET, 20,30", "SET,20 ,30", "SET,1e3,30", "SET,+20,30", "SET,20.,30",
                             "SET,.5,30", "SET,--1,30", "SET,nan,30", "SET,20,inf", "SET,1234567,30"})
    {
        CAPTURE(line);
        CHECK(p.parse(line).kind == CommandKind::Invalid);
    }
    CHECK(p.parse(nullptr).kind == CommandKind::Invalid);
}
