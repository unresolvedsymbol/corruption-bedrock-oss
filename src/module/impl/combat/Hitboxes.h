#pragma once

#include "../../ToggleModule.h"
#include "../../Value.h"
#include <vector>

// Probably going to remove this module. Kinda shit, just use Aura in the ghost preset and fiddle with the attack FOV.
class Hitboxes : public ToggleModule {
	Value<float> size{this, "Size", 3};

public:
	Hitboxes();
};