#pragma once

#include "../../ToggleModule.h"
#include "../../Value.h"

class RideSpeed : public ToggleModule {

public:
	Value<float> factor{this, "Factor", 3.4}, speed{this, "Speed", 0.6}, vertical{this, "Veritcal", 0.4}, glide{this, "Glide", 0};
	Value<bool> absolute{this, "Absolute", false}, control{this, "Control", false}, fly{this, "Fly", false};

	RideSpeed();
};
