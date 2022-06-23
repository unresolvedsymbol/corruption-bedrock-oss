#include <network/protocol/AnimatePacket.h>
#include <network/protocol/RemoveActorPacket.h>
#include <network/protocol/AddPlayerPacket.h>
#include "AntiCheat.h"
#include "network/protocol/MovePlayerPacket.h"
#include "events/impl/PacketEvents.h"

AntiCheat::AntiCheat() : ToggleModule("AntiCheat", false) {
	addDynamicListener<PreReadEvent<MovePlayerPacket>>([&](auto &pe) {
		if (!Util::client->isPlaying() || pe.packet.runtimeID == Util::player->getRuntimeID() || pe.packet.mode != MovePlayerPacket::Mode::Normal)
			return;
		if (auto *player = Util::player->getLevel().getRuntimePlayer(pe.packet.runtimeID)) {
			if (player->ticksExisted < 80)
				return;
			auto delta = pe.packet.pos.subtract(player->prevPos).delta();
			if (!player->onGround && Util::player->getRegion().getMaterial(pe.packet.pos.subtract(0, -2, 0)).isReplaceable()) {
				if (delta > *airSpeed)
					Log() << player->getUnformattedNameTag() << "\u00A7r\u00A77 has violated air speed (" << delta << ")";
				// More air speed logic?
			} else
				if (delta > *groundSpeed)
					Log() << player->getUnformattedNameTag() << "\u00A7r\u00A77 has violated ground speed (" << delta << ")";
				// More ground speed logic?
		}
	});

	// I don't remember exactly what I was going to track with PlayerData
	/*addDynamicListener<PlayerTick>([&](auto &) {
		for (auto &player : players)
			if (player.second.timer > 0)
				player.second.timer--;
	});

	addDynamicListener<PostReadEvent<RemoveActorPacket>>([&](auto &pe) {
		players.erase(pe.packet.uid.data);
	});

	addDynamicListener<PreReadEvent<AddPlayerPacket>>([&](auto &pe) {
		players.emplace(pe.packet.uid.data, PlayerData{});
	});*/

	// Swing happens after the attack in vanilla clients
	addDynamicListener<PreReadEvent<AnimatePacket>>([&](auto &pe) {
		if (!Util::client->isPreGame() && pe.packet.rid != Util::player->getRuntimeID() && pe.packet.action == AnimatePacket::Action::SwingArm && Util::player->hurtTime > 0) { // Self hurt animation
			if (auto *player = Util::player->getLevel().getRuntimePlayer(pe.packet.rid)) {
				auto ourPos = Util::player->pos.add(Util::player->pos.subtract(Util::player->prevPos).multiply(-(*reachPrediction)));
				auto theirPos = player->pos.add(player->pos.subtract(player->prevPos).multiply(*reachPrediction));
				float distance = ourPos.distance(theirPos);
				if (distance > 6) // Can't reliably determine the victim
					return true;
				auto viewDelta = player->prevRotation.distance(Util::getRotationsTo(theirPos, ourPos));
				if (viewDelta.y > *lookYaw)
					Log() << player->getUnformattedNameTag() << "\u00A7r\u00A77 has violated look yaw (" << viewDelta.y << " > " << *lookYaw << ")";
				if (viewDelta.x > *lookPitch)
					Log() << player->getUnformattedNameTag() << "\u00A7r\u00A77 has violated look pitch (" << viewDelta.x << " > " << *lookPitch << ")";
				if (distance > *reach)
					Log() << player->getUnformattedNameTag() << "\u00A7r\u00A77 has violated reach (" << distance << " > " << *reach << ")";
			}
		}
	});
}