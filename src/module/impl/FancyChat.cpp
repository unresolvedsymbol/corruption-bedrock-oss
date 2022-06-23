#include "FancyChat.h"
#include "events/impl/MiscEvents.h"

FancyChat::FancyChat() : ToggleModule("FancyChat") {
	addDynamicListener<ChatEvent>([&](ChatEvent &ce) {
		if (ce.cancelled || ce.text.front() == '/')
			return;

		const char *rainbow = "4c6ea23b9d5";
		int i = 0, ri = 0;
		auto cpy = ce.text;
		for (char c : cpy) {
			if (*doRainbow) {
				ce.text.insert(i, {-62, -89, rainbow[i % 10]});
				i += 3;
			}
			// Fullwidth
			if (*fullWidth && c > 31 && c < 122) {
				// > 95 is lower in fullwidths
				ce.text.replace(i, 1, {-17, static_cast<char>(c > 95 ? -67 : -68), static_cast<char>(c - (c > 95 ? 224 : 160))});
				i += 2;
			}

			i++;
			ri++;
		}
	});
}