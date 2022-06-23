#pragma once

#include "../../ToggleModule.h"
#include "../../Value.h"

class Step : public ToggleModule {
	Value<bool> packets{this, "Packet", true},
		vanilla{this, "Vanilla", false}, // Use vanilla mechanic instead of custom. Doesn't work over 2 blocks.
		reverse{this, "Reverse", true};
	Value<int> maxHeight{this, "Height", 3},
		reverseHeight{this, "Rev Height", 15};

	float lastMinY;

public:
	Step();
};
