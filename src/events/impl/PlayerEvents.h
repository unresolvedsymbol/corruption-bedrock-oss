#pragma once

#include "../Cancellable.h"
#include "math/Vec3.h"
#include "entity/Actor.h"
#include "client/gui/screens/models/ClientInstanceScreenModel.h"

struct MoveInputEvent {};

struct MoveEvent {
	Vec3 &motion;
};

struct RideTick {
	Actor &ride;
};

struct PlayerTick {};

struct FOVEvent {
	float modifier;
};

struct SlowdownEvent : Cancellable {};

struct ChatEvent : Cancellable {
	ClientInstanceScreenModel &screen;
	std::string &text;
};

struct InWaterEvent {
	bool value;
};
