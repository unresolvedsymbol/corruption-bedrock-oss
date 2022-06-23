#pragma once

#include "../../ToggleModule.h"
#include "../../Value.h"

struct Sprint : ToggleModule {
	Sprint();

private:
	Value<bool> multi{this, "Multi", false},
	packet{this, "Packet", true};
};
