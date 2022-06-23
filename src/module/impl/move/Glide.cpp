#include "network/protocol/AnimatePacket.h"
#include "network/protocol/MovePlayerPacket.h"
#include "Glide.h"
#include "events/impl/PacketEvents.h"

Glide::Glide() : ToggleModule("Glide") {
	setOnEnable([&]() {
		stage = -1;
	});

	addModule((new Module("Preset"))->addModule((new Module("CubeCraft"))->setRun([&](auto, args_t) {
		cube = true;
		damage = false;
		h = .5f;
		v = .4f;
		g = .01f;
		vd = 0;
		onEnable();
		Log() << "Loaded CubeCraft Glide";
	// Mineplex has apparently no anticheat
	}))/*->addModule((new Module("Mineplex"))->setRun([&](auto, args_t) {
		cube = damage = false;
		h = .6f;
		v = .4f;
		g = -.1f;
		vd = 10;
		onEnable();
		Log() << "Loaded Mineplex Glide";
	}))*/->addModule((new Module("Vanilla"))->setRun([&](auto, args_t) {
		cube = damage = false;
		h = 2.5f;
		v = .6f;
		g = 0;
		vd = 0;
		onEnable();
		Log() << "Loaded Vanilla Flight";
	})));

	addDynamicListener<MoveEvent>([&](auto &me) {
		auto move = Util::getMoveVec(*h);
		me.motion = move.x;
		me.motion.y = *g;
		me.motion.z = move.y;
		if (*v && --delay < 1) {
			me.motion.y = Util::input->sneak ? -*v : Util::input->jump ? *v : *g;
			delay = *vd;
		}
		Util::player->motion.y = me.motion.y;
	}, EventPriority::High)->addDynamicListener<PreSendEvent<MovePlayerPacket>>([&](auto &pe) {
		// Exploits removed
	}, EventPriority::High);
}
