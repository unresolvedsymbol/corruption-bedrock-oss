#include "Sprint.h"
#include "events/impl/PlayerEvents.h"
#include "events/impl/PacketEvents.h"
#include <network/protocol/PlayerActionPacket.h>

Sprint::Sprint() : ToggleModule("Sprint", true) {
	addDynamicListener<PlayerTick>([&](auto) {
		if ((Util::input->forward > 0 || (*multi && (Util::input->forward || Util::input->strafe))) &&
			!Util::player->collidedHorizontally)
			Util::player->setSprinting(true);
	})->addDynamicListener<PreSendEvent<PlayerActionPacket>>([&](auto &pe) {
		if (!*packet && pe.packet.action == PlayerActionPacket::Action::StartSprinting)
			pe.cancel();
	});
}