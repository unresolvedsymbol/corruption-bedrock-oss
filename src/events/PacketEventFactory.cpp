#include "PacketEventFactory.h"

constexpr bool PacketEventFactory::fireEvent(PacketEventFactory::Type type, Packet &packet) {
	auto fun = map[packet.getId()];
	if (!fun)
		return true;
	return fun(type, packet);
}

#define REG_PACKET(id, type) \
	[id] = &createEvent<type>,

constexpr std::array<bool(*)(PacketEventFactory::Type, Packet &), 164> PacketEventFactory::map{{
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

#undef REG_PACKET