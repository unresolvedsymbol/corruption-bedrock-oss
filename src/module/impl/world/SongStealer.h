#pragma once

#include "../../ToggleModule.h"

struct SongStealer : ToggleModule {
	// Tick -> (Note + Instrument Mask)
	std::vector<std::pair<int, int>> notes{};
	bool started = false;
	int delay = 0, tick = 0;

	SongStealer();
};
