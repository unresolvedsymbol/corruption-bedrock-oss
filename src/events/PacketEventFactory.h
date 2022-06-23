#pragma once

#include <array>
#include "events/impl/PacketEvents.h"
#include "events/EventChannel.h"
#include "network/protocol/TextPacket.h"
#include "network/protocol/AnimatePacket.h"
#include "network/protocol/PlayerActionPacket.h"
#include "network/protocol/LevelSoundEventPacket.h"
#include "network/protocol/MobEffectPacket.h"
#include "network/protocol/AddItemActorPacket.h"
#include "network/protocol/AddActorPacket.h"
#include "network/protocol/ActorFallPacket.h"
#include "network/protocol/LevelEventPacket.h"
#include "network/protocol/DisconnectPacket.h"
#include "network/protocol/MovePlayerPacket.h"
#include "network/protocol/ActorEventPacket.h"
#include "network/protocol/AddPlayerPacket.h"
#include "network/protocol/RemoveActorPacket.h"
#include "network/protocol/SetActorDataPacket.h"
#include "network/protocol/InteractPacket.h"
#include "network/protocol/InventoryTransactionPacket.h"
#include "network/protocol/SetActorMotionPacket.h"
#include "network/protocol/MoveActorAbsolutePacket.h"
#include "network/protocol/PlaySoundPacket.h"
#include "network/protocol/UpdateBlockPacket.h"

#define REG_PACKET(id, type) \
	[id] = &createEvent<type>,

// Parameterized packet event factory!
class PacketEventFactory {
public:
	// TODO: Use typeinfo hash instead? more fucky wucky shit
	enum Type : uint8_t {
		PreSend,
		PostSend,
		PreRead,
		PostRead
	};

private:
	// Returns !cancelled
	template<typename T> static inline bool createEvent(Type type, Packet &packet) {
		switch (type) {
			case PreSend:
				return EventChannel::fire<PreSendEvent<T>>(reinterpret_cast<T&>(packet));
			case PreRead:
				return EventChannel::fire<PreReadEvent<T>>(reinterpret_cast<T&>(packet));
			case PostSend:
				return EventChannel::fire<PostSendEvent<T>>(reinterpret_cast<T&>(packet));
			case PostRead:
				return EventChannel::fire<PostReadEvent<T>>(reinterpret_cast<T&>(packet));
		}
		return true;
	}

	static constexpr std::array<bool(*)(Type, Packet &), 164> map{{
			REG_PACKET(0x5, DisconnectPacket)
			REG_PACKET(0x9, TextPacket)
			REG_PACKET(0x13, MovePlayerPacket)
			REG_PACKET(0x1e, InventoryTransactionPacket)
			REG_PACKET(0x21, InteractPacket)
			REG_PACKET(0x24, PlayerActionPacket)
			REG_PACKET(0x25, ActorFallPacket)
			REG_PACKET(0x7B, LevelSoundEventPacket)
			REG_PACKET(0x0C, AddPlayerPacket)
			REG_PACKET(0x0D, AddActorPacket)
			REG_PACKET(0x0E, RemoveActorPacket)
			REG_PACKET(0x0F, AddItemActorPacket)
			REG_PACKET(0x12, MoveActorAbsolutePacket)
			REG_PACKET(0x15, UpdateBlockPacket)
			REG_PACKET(0x19, LevelEventPacket)
			REG_PACKET(0x1B, ActorEventPacket)
			REG_PACKET(0x1C, MobEffectPacket)
			REG_PACKET(0x27, SetActorDataPacket)
			REG_PACKET(0x28, SetActorMotionPacket)
			REG_PACKET(0x2C, AnimatePacket)
			REG_PACKET(0x56, PlaySoundPacket)
	}};
public:

	static constexpr bool fireEvent(Type type, Packet &packet) {
		auto fun = map[packet.getId()];
		if (!fun)
			return true;
		return fun(type, packet);
	}
};

#undef REG_PACKET
