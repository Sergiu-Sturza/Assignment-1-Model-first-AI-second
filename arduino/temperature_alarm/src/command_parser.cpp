#include "tempalarm/command_parser.hpp"

#include <string.h>

namespace tempalarm {

namespace {

const char setPrefix[] = "SET,";
const int maxDigits = 6;   // per side of the point: far beyond any temperature, no overflow

bool isDigit(char c) { return c >= '0' && c <= '9'; }

/// Reads one decimal number at @p text that ends at @p terminator: an optional minus,
/// digits, and optionally a point and digits ("-12.5", "30") — the syntax protocol.js
/// writes, and nothing else (no spaces, exponents, "nan" or "inf"). Returns false if
/// there is none. Hand-written instead of strtod(): stricter, faster, and smaller on AVR.
bool number(const char* text, char terminator, float& value, const char*& rest)
{
    const char* p = text;
    const bool negative = *p == '-';
    if (negative)
    {
        ++p;
    }
    long whole = 0;
    int digits = 0;
    for (; isDigit(*p); ++p)
    {
        if (++digits > maxDigits)
        {
            return false;
        }
        whole = whole * 10 + (*p - '0');
    }
    if (digits == 0)
    {
        return false;
    }
    long fraction = 0;
    long scale = 1;
    if (*p == '.')
    {
        ++p;
        digits = 0;
        for (; isDigit(*p); ++p)
        {
            if (++digits > maxDigits)
            {
                return false;
            }
            fraction = fraction * 10 + (*p - '0');
            scale *= 10;
        }
        if (digits == 0)
        {
            return false;
        }
    }
    if (*p != terminator)
    {
        return false;
    }
    const float magnitude = static_cast<float>(whole) + static_cast<float>(fraction) / static_cast<float>(scale);
    value = negative ? -magnitude : magnitude;
    rest = p;
    return true;
}

}  // namespace

Command CommandParser::parse(Text line) const
{
    Command command = {CommandKind::Invalid, 0.0f, 0.0f};
    if (line == nullptr)
    {
        return command;
    }
    if (strcmp(line, "GET") == 0)
    {
        command.kind = CommandKind::GetThreshold;
        return command;
    }
    if (strncmp(line, setPrefix, sizeof setPrefix - 1) != 0)
    {
        return command;
    }
    const char* rest = nullptr;
    float low = 0.0f;
    float high = 0.0f;
    if (number(line + sizeof setPrefix - 1, ',', low, rest) && number(rest + 1, '\0', high, rest))
    {
        command.kind = CommandKind::SetThreshold;
        command.low = low;
        command.high = high;
    }
    return command;
}

}  // namespace tempalarm
