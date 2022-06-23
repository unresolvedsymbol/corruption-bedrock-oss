#pragma once

#include "../../ToggleModule.h"

struct NoteBot : ToggleModule {
	// Tick -> (Note + Instrument Mask)
	std::unordered_multimap<int, int> notes;
	int tick = 0, end = 0;

	NoteBot();
};
