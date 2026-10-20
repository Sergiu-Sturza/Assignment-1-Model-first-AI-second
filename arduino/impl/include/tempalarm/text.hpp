#pragma once

// Text types shared by the classes. Typedefs, not classes, so they add nothing to the
// class diagram. Fixed-size buffers, no heap: the same code runs on a 2 KB-RAM Arduino.
//
// Headers include each other by file name only ("text.hpp"), so they also resolve when
// tools/sync_firmware.py copies them into the sketch's src/ folder.

#include <stddef.h>

namespace tempalarm {

/// A zero-terminated string the callee does not own.
typedef const char* Text;

/// The longest command line SerialLink accepts, without the newline.
const size_t maxLineLength = 31;
/// The buffer one received line is assembled in.
typedef char LineText[maxLineLength + 1];

/// The longest line MessageFormatter writes, without the newline.
const size_t maxMessageLength = 39;
/// The buffer one outgoing message is formatted in.
typedef char MessageText[maxMessageLength + 1];

}  // namespace tempalarm
