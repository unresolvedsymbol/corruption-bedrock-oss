#pragma once

#include "../../ToggleModule.h"
#include "../../Value.h"

struct NoFall : ToggleModule {
	NoFall();

	Value<bool> onGround{this, "OnGround", true},
		elytra{this, "Elytra", false}; // Fake gliding before hitting the ground to reset fall distance (Nukkit/TeaSpoon NoFall!)
		// TODO: nukkit nofall, tp up the falldistance before hitting the ground or just always offset .75 vertically lol
};
