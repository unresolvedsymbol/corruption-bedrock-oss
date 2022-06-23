#include <network/protocol/AddItemActorPacket.h>
#include <network/protocol/RemoveActorPacket.h>
#include <network/protocol/LevelEventPacket.h>
#include <network/protocol/ActorEventPacket.h>
#include <network/protocol/PlayerActionPacket.h>
#include <network/protocol/AddPlayerPacket.h>
#include "AntiCrash.h"
#include "events/impl/PacketEvents.h"
#include <string>
#include <util/DeviceOS.h>

AntiCrash::AntiCrash() : ToggleModule("AntiCrash", true) {
	addDynamicListener<PreReadEvent<AddItemActorPacket>>([&](auto &pe) {
		if (pe.packet.fromFishing) // We need to see what we got!
			return;
		if (items.size() > *maxItems || pe.packet.stack.isNull()) {
			pe.cancel();
			return;
		}
		// Blocks usually aren't important items (diamonds, swords, armor etc)
		// Wat
		if (pe.packet.stack.isBlock()) { // Check if valuable block? eg diamond
			pe.cancel();
			return;
		}
		//items.emplace(pe.packet.uid.data);
		for (auto entry : items) {
			if (entry.second.item == pe.packet.stack.getId() && entry.second.pos.distance(pe.packet.pos) < 3) {
				// TODO: Increase near count?
				pe.cancel();
				return;
			}
		}
		items.emplace(pe.packet.uid.data, SavedItem{pe.packet.stack.getId(), pe.packet.pos});
	})->addDynamicListener<PreReadEvent<AddPlayerPacket>>([](auto &pe) {
		/*static void *playerPktCpy = nullptr;
		if (!playerPktCpy) {
			playerPktCpy = malloc(sizeof(AddPlayerPacket));
			std::memcpy(playerPktCpy, reinterpret_cast<void *>(&const_cast<AddPlayerPacket &>(pe.packet)), sizeof(AddPlayerPacket));
			Log() << "copied an add player packet addr @ " << std::hex << playerPktCpy;
		}*/
		/*if (pe.packet.platform != DeviceOS::Unknown) {
			//gsl::czstring_span<> &xd = pe.packet.username;
			if (pe.packet.username.empty())
				return;
			Log() << gsl::to_string(pe.packet.username) << "\u00A7r\u00A77 is on platform "
				  << static_cast<uint32_t>(pe.packet.platform);

		}*/
		//if (!pe.packet.username.empty())
		//	Log() << gsl::to_string(pe.packet.username);
	})->addDynamicListener<PreReadEvent<RemoveActorPacket>>([&](auto &pe) {
		items.erase(pe.packet.uid.data);
	})->addDynamicListener<PreReadEvent<LevelEventPacket>>([](auto &pe) {
		if (pe.packet.event == 2001)
			pe.cancel(); // Block destroy particles
	})->addDynamicListener<PreSendEvent<Packet>>([&](auto &pe) {
		if (pe.packet.getId() == 0x1) // LoginPacket
			items.clear(); // Clear items on login to another server
	})->/*addDynamicListener<PreReadEvent<Packet>>([&](auto &pe) {
		if (pe.packet.getId() == 0x16) // AddPaintingPacket fuck em anyway (check motive in future)
			pe.cancel();
	})->*/addDynamicListener<PreReadEvent<ActorEventPacket>>([](auto &pe) {
		if (pe.packet.event == 57 || // All eat particles, they're laggy ._.
			pe.packet.event == 38 // Dust particles?
		)
			pe.cancel();
	})->addDynamicListener<PreReadEvent<PlayerActionPacket>>([](auto &pe) {
		// Sprint particles, stopped fucking working for some god damn reason
		if (pe.packet.action == PlayerActionPacket::Action::StartSprinting)
			pe.cancel();
	});

	EventChannel::registerStaticListener<PreReadEvent<Packet>>([&](auto &pe) {
		// Temporary item clear after world change on server. Not perfect. Map PlayerListPacket later then remove on list removall
		if (pe.packet.getId() == 0x37) // AdventureSettingsPacket
			items.clear();
	});

	EventChannel::registerStaticListener<PreSendEvent<Packet>>([&](auto &pe) {
		if (pe.packet.getId() == 0x1) // LoginPacket
			items.clear();
	});
}