#pragma once

#include "../../ToggleModule.h"
#include "../../Value.h"

struct Scaffold : ToggleModule {
	Scaffold();

private:
	Value<bool> silent{this, "Silent", true},
		tower{this, "Tower", true};
};