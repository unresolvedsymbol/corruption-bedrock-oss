#pragma once

#include "../ToggleModule.h"
#include "../BoundValue.h"

struct AntiCheat : ToggleModule {
	Value<float> airSpeed{this, "Air Speed", 2.6f}, // 2.4?
		groundSpeed{this, "Ground Speed", 1.8f},
		lookYaw{this, "Yaw", 90.f},
		lookPitch{this, "Pitch", 50.f},
		reach{this, "Reach", 4.f},
		reachPrediction{this, "Interpolation", 3.f};

	struct PlayerData {
		unsigned char score = 0, timer = 60;
	};

	std::unordered_map<uint64_t, PlayerData> players;

	AntiCheat();
};
