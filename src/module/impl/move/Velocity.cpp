#include <network/protocol/SetActorMotionPacket.h>
#include "Velocity.h"
#include "events/impl/PacketEvents.h"

Velocity::Velocity() : ToggleModule("Velocity", true) {
	addDynamicListener<PreReadEvent<SetActorMotionPacket>>([&](auto &pe) {
		if (*debug) {
			auto *player = Util::player->getLevel().getRuntimePlayer(pe.packet.rid);
			if (player && player->hurtTime == 6)
				Log() << player->getNameTag() << " took " << pe.packet.motion;
		}
		// Server sends the expected kb to players, not what they actually took so pointless.
		if (Util::player && pe.packet.rid != Util::player->getRuntimeID())
			return;

		// TODO: Anti Combo? Only reduce knockback if still in air or hit again quickly

		if (*cancel) {
			pe.cancel();
			return;
		}

		if (*chance || rd(re) < *chance)
			pe.packet.motion = pe.packet.motion.multiply(*h, *v);

		if (*delay && (*delayChance || rd(re) < *delayChance)) {
			queue.insert({Util::player->ticksExisted + *delay, pe.packet.motion});
			pe.cancel();
		}
	});

	addDynamicListener<PlayerTick>([&](auto &) {
		auto it = queue.begin();
		while (it != queue.end()) {
			if (it->first <= Util::player->ticksExisted) {
				Util::player->lerpMotion(it->second);
				Util::player->motion = Util::player->motion.add(it->second);
				it = queue.erase(it);
			} else ++it;
		}
	});
}
