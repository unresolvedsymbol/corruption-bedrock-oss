#pragma once

#include "../../ToggleModule.h"
#include "../../Value.h"
#include "../../BoundValue.h"

struct AutoHeal : ToggleModule {
	BoundValue<int> health{this, "Health", 10, 1, 20};
	Value<int> wait{this, "Delay", 5},
		hold{this, "Hold Wait", 1};
	Value<bool> splash{this, "Splash", true},
		bottle{this, "Bottle", true},
		up{this, "Splash Up", true},
		jump{this, "Jump", false},
		clip{this, "Clip", false};
	Value<bool> gapple{this, "Gapple", true},
		soup{this, "Soup", true};

	short slot, oldSlot;
	ContainerItemStack *stack, *oldStack;
	unsigned char delay = 0, stage = 0, slow = 0, mode = 0;

	AutoHeal();

	bool validEffects(short data);
};
