#include "tico/TicoKeyboard.h"

#include "ppsspp_config.h"

#if PPSSPP_PLATFORM(SWITCH)

#include <switch.h>

#include <vector>

namespace Tico {

bool ShowKeyboard(const char *headerText, const char *initialText,
                  size_t maxLength, std::string *out) {
	if (!out || maxLength == 0) {
		return false;
	}

	SwkbdConfig kbd;
	if (R_FAILED(swkbdCreate(&kbd, 0))) {
		return false;
	}

	swkbdConfigMakePresetDefault(&kbd);
	if (headerText && headerText[0]) {
		swkbdConfigSetHeaderText(&kbd, headerText);
	}
	if (initialText && initialText[0]) {
		swkbdConfigSetInitialText(&kbd, initialText);
	}
	swkbdConfigSetStringLenMax(&kbd, (u32)maxLength);

	// UTF-8 runs up to 4 bytes per code point, plus room for the terminator.
	std::vector<char> buffer(maxLength * 4 + 1, '\0');
	const Result rc = swkbdShow(&kbd, buffer.data(), buffer.size());
	swkbdClose(&kbd);

	// swkbdShow also fails when the user backs out, which is not an error here.
	if (R_FAILED(rc)) {
		return false;
	}

	buffer.back() = '\0';
	out->assign(buffer.data());
	return !out->empty();
}

}  // namespace Tico

#else

namespace Tico {

bool ShowKeyboard(const char *, const char *, size_t, std::string *) {
	return false;
}

}  // namespace Tico

#endif
