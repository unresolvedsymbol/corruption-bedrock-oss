#pragma once

#include "../../ToggleModule.h"

struct BowAim : ToggleModule {
	BowAim();

private:
	// Test prediction stuff
	Value<float> sprintFactor{this, "Sprint Factor", 1.5},
		usingFactor{this, "Using Factor", 0.7},
		normalFactor{this, "Factor", 1},
		gravity{this, "Gravity", 0.006f}; // i think 0.004 on steadfast and apparently 0.012 on nukkit
	Value<bool> distance{this, "Distance", true};

	float calcTraj(float horiz, float vert, float vel);;
};
