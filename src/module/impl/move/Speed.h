#pragma once

#include "../../ToggleModule.h"
#include "../../Value.h"
#include <random>

class Speed : public ToggleModule {
	Value<float> min{this, "Base", 1.f},
		boost{this, "Boost", 0.f},
		hop{this, "Hop", .4f},
		multiplier{this, "Multiplier", 1.2f},
		glideSpeed{this, "Glide Sp", 1.7f},
		max{this, "Limit", 0.f},
		friction{this, "Friction", .66f},
		lowHopMot{this, "Low Hop Motion", -.3f},
		packetOffset{this, "Packet Offset", .4f};
	Value<int> lowHop{this, "Low Hop Tick", 0},
		warm{this, "Warm Up", 2},
		resetTick{this, "Reset Tick", 6},
		glideTick{this, "Glide Tick", 0},
		packetTick{this, "Packet Tick", 0};
	Value<bool> test{this, "Debug", false},
		longJump{this, "Long Jump", false},
		absolute{this, "Absolute Multiplier", false},
		randomize{this, "Randomize", true};
	// TODO: Auth packets
		//authentication{this, "Authentication", false};

	int stage, // Current tick
		warmups; // Warm up hops left
	float moveSpeed, lastSpeed; // Current/last horizontal speed

	Vec3 hopStartPos;

	std::default_random_engine re;
	std::uniform_real_distribution<float> rd{0.f, .01f};

public:
	Speed();
};