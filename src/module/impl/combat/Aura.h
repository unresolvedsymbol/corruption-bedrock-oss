#pragma once

#include "../../Value.h"
#include "../../BoundValue.h"
#include "../../ToggleModule.h"
#include <random>
#include <unordered_set>

class Aura : public ToggleModule {
	std::unordered_set<uint64_t> playersSwung;
	long long int nextDelay = 100;
	std::chrono::time_point<std::chrono::system_clock> lastTime = std::chrono::system_clock::now();
	bool otherSwing;
	Value<bool> antiBot{this, "Anti Bot", true},
		multi{this, "Multi", false},
		silentAim{this, "Silent AimBot", false}, // Some servers still check angles within 180 degrees (since we're spoofing a mobile/touch device)
		inventory{this, "Inventory", false},
		swing{this, "Swing", true},
		slow{this, "Slow", false},
		legacy{this, "Legacy", false}, // Steadfast hack...
		teleport{this, "TP", false},
		teleportReturn{this, "TP Back", false},
		blockRaytrace{this, "Block Ray", false},
		requireTool{this, "Tool", true},
		autoTool{this, "Auto Tool", false},
		fastHit{this, "Fast Hit", false},
		tick{this, "Tick", false},
		unsprint{this, "Unsprint", true}; // Critical bypass
	Value<float> range{this, "Range", 5},
		swingRange{this, "Swing Range", 6},
		teleportSpeed{this, "TP Speed", 3};
	BoundValue<int> attacks{this, "Attacks", 1, 0, 20},
		speed{this, "Speed", 7, 0, 20},
		swingSpeed{this, "Swing Speed", 5, 0, 20},
		targets{this, "Targets", 3, 0, 50},
		fov{this, "FOV", 360, 0, 360},
		swingFov{this, "Swing FOV", 360, 0, 360},
		teleportRelative{this, "TP Relative", 0, -8, 8},
		teleportRange{this, "TP Range", 6, 0, 15};

	std::default_random_engine re;
	std::uniform_real_distribution<float> rd{-1.f, 1.f};

public:
	Aura();
};

