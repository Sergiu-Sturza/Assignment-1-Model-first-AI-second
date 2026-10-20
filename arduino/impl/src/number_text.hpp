#pragma once

// Internal to the library: the text building the formatter and the display share.
// Lives in src/, not in include/, so it is not part of the compared design. avr-libc's
// printf has no %f, so numbers are written by hand.

#include <stddef.h>

namespace tempalarm {
namespace detail {

inline bool isNotANumber(float value) { return value != value; }

/// Appends @p text to the zero-terminated @p out of @p capacity bytes, which holds
/// @p length characters, cutting it off when full. Returns the new length.
inline size_t append(char* out, size_t capacity, size_t length, const char* text)
{
    while (*text != '\0' && length + 1 < capacity)
    {
        out[length++] = *text++;
    }
    out[length] = '\0';
    return length;
}

/// Appends @p value with one decimal, rounded half away from zero ("23.4", "-5.0").
/// Values beyond +-99999 are written as that limit.
inline size_t appendDecimal(char* out, size_t capacity, size_t length, float value)
{
    const float limit = 99999.0f;
    value = value > limit ? limit : (value < -limit ? -limit : value);
    const long tenths = static_cast<long>(value * 10.0f + (value < 0.0f ? -0.5f : 0.5f));
    unsigned long magnitude = tenths < 0 ? 0UL - static_cast<unsigned long>(tenths)
                                         : static_cast<unsigned long>(tenths);
    char reversed[12];
    size_t n = 0;
    reversed[n++] = static_cast<char>('0' + magnitude % 10);
    magnitude /= 10;
    reversed[n++] = '.';
    do
    {
        reversed[n++] = static_cast<char>('0' + magnitude % 10);
        magnitude /= 10;
    } while (magnitude != 0);
    if (tenths < 0)
    {
        reversed[n++] = '-';
    }
    char digits[13];
    for (size_t i = 0; i < n; ++i)
    {
        digits[i] = reversed[n - 1 - i];
    }
    digits[n] = '\0';
    return append(out, capacity, length, digits);
}

}  // namespace detail
}  // namespace tempalarm
