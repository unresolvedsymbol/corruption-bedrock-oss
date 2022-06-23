#pragma once

#include "../../ToggleModule.h"
#include "../../Value.h"

class Nuker : public ToggleModule {
	Value<int> range{this, "Range", 6}, count{this, "Blocks", 3};
	Value<bool> bedrock{this, "Bedrock", true}; // Should try to destroy bedrock or not
	bool tick;

public:
	Nuker();
};

