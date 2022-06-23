#pragma once

#include "../../ToggleModule.h"
#include "../../Value.h"

struct Phase : ToggleModule {
	Phase();

private:
	Value<float> distance{this, "Distance", 1.1f},
		vertical{this, "Vertical", -.03f}; // .05 .06 can also help on some servers
	Value<bool> sneak{this, "Sneak", true}; // Just like HCF days
};
