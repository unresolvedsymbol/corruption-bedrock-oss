#pragma once

#include "../../ToggleModule.h"
#include "../../Value.h"

class Glide : public ToggleModule {
	Value<float> h{this, "Horizontal", 0.3}, // Horizontal speed
		v{this, "Vertical", 0.2}, // Vertical speed
		g{this, "Glide Speed", 0}, // Glide speed
		vd{this, "VD", 0}; // Vertical delay
	Value<bool> damage{this, "Damage", false},
		cube{this, "CubeCraft", false};
	int stage = 0, delay = 0;

public:
	Glide();
};