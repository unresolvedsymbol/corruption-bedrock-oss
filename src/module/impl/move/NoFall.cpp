#include <network/protocol/MovePlayerPacket.h>
#include <network/protocol/ActorFallPacket.h>
#include <network/protocol/PlayerActionPacket.h>
#include "NoFall.h"
#include "events/impl/PacketEvents.h"

NoFall::NoFall() : ToggleModule("NoFall", true) {
	addDynamicListener<PreSendEvent<MovePlayerPacket>>([&](auto &e) {
		e.packet.onGround = *onGround;
		
		// Elytra exploit removed
	}, EventPriority::Low)->addDynamicListener<PreSendEvent<ActorFallPacket>>([](auto &e) {
		e.cancel();
	});
}
