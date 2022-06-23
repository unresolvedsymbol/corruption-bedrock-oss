#pragma once

#include "world/VanillaBlocks.h"
#include "../../ToggleModule.h"
#include "../../BoundValue.h"
#include <unordered_set>

struct Wallhack : ToggleModule {
	Wallhack();

private:
	BoundValue<float> brightness{this, "Brightness", .7, 0, 1},
		opacity{this, "Opacity", .25, 0, 1};

	bool wants(const Block &);
};
