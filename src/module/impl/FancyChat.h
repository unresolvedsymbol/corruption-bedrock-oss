#pragma once

#include "../ToggleModule.h"
#include "../Value.h"

struct FancyChat : ToggleModule {
	FancyChat();

private:
	Value<bool> doRainbow{this, "Rainbow", false},
		fullWidth{this, "Fullwidth", true};
};
