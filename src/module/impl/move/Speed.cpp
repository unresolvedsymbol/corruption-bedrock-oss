#include <network/protocol/MovePlayerPacket.h>
#include <network/protocol/PlayerActionPacket.h>
#include <chrono>
#include "Speed.h"
#include "events/impl/PlayerEvents.h"
#include "events/impl/PacketEvents.h"
#include "Corruption.h"

Speed::Speed() : ToggleModule("Speed") {
	setOnEnable([&]() {
		moveSpeed = stage = 0;
		warmups = *warm;
	});

	// You guys are lucky, here's my speed exploits (that probably don't work anymore)
	addModule((new Module("Preset"))->addModule(((new Module("LBGlide"))->setRun([&](auto, args_t) {
		longJump = packetTick = boost = max = warm = 0;
		min = 1; // 1.45
		hop = .4;
		lowHop = false;
		absolute = true;
		multiplier = 1; // 2.45
		friction = .66;
		resetTick = 6;
		glideTick = 17;
		glideSpeed = 1.75;
		onEnable();
		Log() << "Loaded Lifeboat Glide";
	})))->addModule((new Module("LBHop"))->setRun([&](auto, args_t) {
		packetTick = longJump = glideTick = boost = lowHop = absolute = false;
		min = 1.3; // TODO: Tweak
		max = 2.16;
		hop = .3998;
		multiplier = 1.64; // 1.65 gets .62~ at best, kicks sometimes though...
		friction = .66;
		resetTick = 6;
		warm = 2; // this is important! the multiplier/friction will become out of balance without it
		onEnable();
		Log() << "Loaded Lifeboat Hop";
	}))->addModule((new Module("LBOnGround"))->setRun([&](auto, args_t) {
		max = boost = longJump = glideTick = warm = hop = packetTick = 0;
		min = 1;
		multiplier = 1.74; // ~.6
		friction = .99;
		// TODO: Check hunger!!!
		resetTick = 4;
		absolute = true;
		onEnable();
		Log() << "Loaded Lifeboat Ground";
	}))->addModule((new Module("CubeGround"))->setRun([&](auto, args_t) {
		packetTick = longJump = glideTick = boost = hop = max = warm = 0;
		min = 1;
		lowHop = false;
		absolute = false;
		multiplier = 6; // You can fucking use any arbitrary speed with a friction close to 1... 0.99 being least error prone
		friction = .99; // This is the magic
		resetTick = 4;
		onEnable();
		Log() << "Loaded Cubecraft Ground";
	}))->addModule((new Module("CubeHop"))->setRun([&](auto, args_t) {
		// This mode is pointless, flags much more than onGround and doesn't work with criticals
		lowHop = absolute = longJump = packetTick = glideTick = max = warm = 0;
		min = 1;
		boost = .99;
		hop = .4;
		multiplier = 2.9; // 3.3
		friction = .66;
		resetTick = 6;
		onEnable();
		Log() << "Loaded Cubecraft Hop";
	}))->addModule((new Module("CubeLowHop"))->setRun([&](auto, args_t) {
		packetTick = glideTick = longJump = max = warm = absolute = false;
		// Another pointless mode
		min = 1;
		boost = 1.35;
		hop = .4;
		lowHop = true;
		multiplier = 2;
		// 1.35 * 2 = 2.7 while
		// .99 * 2.9 = 2.871 so idk if this is slightly faster or slower cause it's doing more hops and the boost tick
		friction = .66;
		resetTick = 6;
		onEnable();
		Log() << "Loaded Cubecraft LowHop";
	}))->addModule((new Module("VanillaHop"))->setRun([&](auto, args_t) { // Also hive speed w packetloss and disable hop (.sp p v; sp h s 0)
		boost = max = warm = longJump = lowHop = packetTick = glideTick = 0;
		min = 1;
		hop = .4;
		absolute = true;
		multiplier = 3.4;
		friction = .5;
		resetTick = 5;
		onEnable();
		Log() << "Loaded Vanilla Hop";
	}))->addModule((new Module("Long"))->setRun([&](auto, args_t) {
		max = warm = longJump = lowHop = packetTick = glideTick = 0;
		min = 1;
		boost = 3;
		hop = .424;
		multiplier = 2.149802;
		friction = .66;
		resetTick = 6;
		onEnable();
		Log() << "Loaded LongJump";
	}))->addModule((new Module("Hyperlands"))->setRun([&](auto, args_t) {
		max = warm = packetTick = glideTick = lowHop = 0;
		min = 1;
		boost = 4;
		hop = .424;
		multiplier = 2.149;
		longJump = true;
		friction = .6;
		resetTick = 20;
		onEnable();
		Log() << "Loaded Hyperlands LongJump";
	}))->addModule((new Module("Hive"))->setRun([&](auto, args_t) {
		max = warm = packetTick = glideTick = longJump = boost = 0;
		lowHop = 1;
		multiplier = 1.9;
		friction = .66;
		resetTick = 6;
		hop = .3998;
		absolute = false;
		Log() << "Loaded Hive Low Hop";
	})));

	addDynamicListener<PreSendEvent<MovePlayerPacket>>([&] (auto &pe) {
		if (*glideTick && stage >= *glideTick)
			pe.packet.onGround = true;

		if (warmups)
			return;

		if (*packetTick && stage == *packetTick)
			pe.packet.y += *packetOffset;
	}, EventPriority::Lowest);

	addDynamicListener<MoveEvent>([&](MoveEvent &event) {

		if (Util::client->getInput().isInWater() || (Util::player->isUsingItem() && !Corruption::get()->modules().get<NoSlow>().get())) {
			// TODO: Combine water speed, do this in Util::getBaseMoveSpeed() ?
			stage = 0;
			warmups = *warm;
			return;
		}

		if (*packetTick && !Util::player->getRegion().getMaterial(BlockPos{Util::player->getPos()}.neighbor(Direction::UP)).isReplaceable()) {
			warmups = 4;
			stage = 0;
			return;
		}

		if ((Util::input->forward == 0 && Util::input->strafe == 0) && (!*glideTick || stage < *glideTick)) {
			event.motion.x = event.motion.z = stage = 0;
			warmups = *warm;
			return;
		}

		// If restarting and not on ground don't start
		if (stage < 2 && !Util::player->onGround)
			return;

		auto base = Util::getBaseMoveSpeed();
		if (*randomize)
			base -= rd(re);
		switch (stage++) {
			case 1:
				// Initial boost
				moveSpeed = *boost * base;
				break;
			case 2:
				if (*hop) {
					Util::player->motion.y = event.motion.y = *hop;
					Util::sendPacket(PlayerActionPacket{Util::player->getRuntimeID(), PlayerActionPacket::Action::Jump}, true);
				}

				// Hop multiplier
				if (*absolute)
					moveSpeed = *multiplier * base;
				else
					moveSpeed *= *multiplier;

				if (*test)
					Log() << "Hop speed " << moveSpeed;

				// Deplete warmup hops
				if (warmups)
					warmups--;
				break;
			case 3:
				hopStartPos = Util::player->getPos();

				// Initial friction
				moveSpeed = lastSpeed - (*friction * (lastSpeed - Util::getBaseMoveSpeed()));
				break;
			default:
				// Friction
				moveSpeed = lastSpeed - lastSpeed / 159;
				break;
		}

		// If finished onGround and has reached resetTick
		if ((!*hop || (Util::player->onGround && Util::player->collidedVertically)) && stage >= *resetTick) {
			stage = *boost ? 1 : 2;
			if (*test) {
				auto pos = Util::player->pos;
				float xD = pos.x - hopStartPos.x, zD = pos.z - hopStartPos.z;
				Log() << "Hop dist " << std::sqrt(xD * xD + zD * zD);
			}
		}

		if (*longJump && stage > 6 && event.motion.y < 0) {
			for (int i = 0; i < 5; ++i)
				if (!Util::player->getRegion().getMaterial(Util::player->aabb.min.add(event.motion.x * 1.2, event.motion.y - i, event.motion.y * 1.2)).isReplaceable()) {
					Util::player->motion.y = event.motion.y = -1;
					event.motion.x = event.motion.z = 0;
					return;
				}

			// Glide somewhat
			if (event.motion.y < -0.04)
				event.motion.y = -0.04;
			moveSpeed += 0.15; // Keep gettin' fucking faster as you don't reach whatever land
		}

		if (*lowHop && stage == *lowHop)
			Util::player->motion.y = event.motion.y = *lowHopMot;

		if (*glideTick && stage >= *glideTick) {
			Util::player->motion.y = event.motion.y = -0.001;
			moveSpeed = Util::getBaseMoveSpeed() * (*glideSpeed - 0.01);
		}

		if (!warmups) {
			// Caps
			moveSpeed = std::max(moveSpeed, *min * base);
			if (*max)
				moveSpeed = std::min(moveSpeed, *max * base);

			auto move = Util::getMoveVec(moveSpeed);
			event.motion.x = move.x;
			event.motion.z = move.y;
		}

		lastSpeed = moveSpeed;

		if (*test)
			Log() << "Current speed: " << moveSpeed;
	});
}
