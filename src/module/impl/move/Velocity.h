#pragma once

#include "../../ToggleModule.h"
#include "../../Value.h"
#include "../../BoundValue.h"
#include <random>

class Velocity : public ToggleModule {
	Value<bool> cancel{this, "Cancel", true}, debug{this, "Debug", false};
	Value<float> v{this, "Vertical", 1}, h{this, "Horizontal", 0.8};
	Value<unsigned int> delay{this, "Delay", 0}; // Fucking blatant as hell why do people use this? (especially when I actually have 30ms)
	BoundValue<float> chance{this, "Chance", 0.25, 0, 1}, delayChance{this, "Delay Chance", 0.15, 0, 1};

	std::default_random_engine re;
	std::uniform_real_distribution<float> rd{0.f, 1.f};

	std::unordered_map<unsigned int, Vec3> queue;

public:
	Velocity();
};