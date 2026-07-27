#pragma once

#include <string>

namespace Tico {

// Opens the Switch software keyboard and blocks until the user confirms or
// cancels. Only safe to call from the main thread, and only while emulation is
// paused: the keyboard is a system applet that takes over the foreground.
//
// Returns false if the user cancelled or the applet could not be opened;
// `out` is left untouched in that case.
bool ShowKeyboard(const char *headerText, const char *initialText,
                  size_t maxLength, std::string *out);

}  // namespace Tico
