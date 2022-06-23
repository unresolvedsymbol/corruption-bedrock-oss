#pragma once

#include "../../ToggleModule.h"
#include "../../Value.h"
#include "../../BoundValue.h"
#include <random>

struct AimBot : ToggleModule {
	AimBot();

private:
	Value<float> range{this, "Range", 4.75}; // Range from player to be considered a target
	BoundValue<int> fov{this, "FOV", 60, 1, 360}; // Distance from cursor for a player to be considered a target
	BoundValue<float> speed{this, "Smoothness", 9, 1, 10}, // Divising delta factor (speed up when further from cursor)
		base{this, "Base", 0.7, 0, 10}, // Minimum speed
		min{this, "Min Delta", 3, 0, 30}; // Minimum delta required
	BoundValue<int> randSpeed{this, "Random Speed", 5, 0, 20}; // Frequency of randomizing target point on hitbox
	BoundValue<float> width{this, "Width", 2, 0, 2}, // Max hitbox width in randomization
		height{this, "Height", 1.35, 0, 2}; // Max hitbox height in randomization

	char randDelay = 0;
	float randWidth1 = 0, randWidth2 = 0, randHeight = 0, targetWidth = 0, targetHeight = 0;
	Actor *target = nullptr;

	std::default_random_engine re;
	std::uniform_real_distribution<float> rd{0.f, 1.f};
};
