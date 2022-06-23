#pragma once

#include <util/serialize.h>
#include "network/protocol/Packet.h"

struct AttackPacket : Packet {
	int64_t entityID;
	int slot;

	AttackPacket(int64_t entityID, int slot) : entityID(entityID), slot(slot) {};

	virtual int getId() const {
		return 0x1e;
	};

	virtual std::string getName() const {
		return "InventoryTransactionPacket";
	};

	virtual void write(BinaryStream &stream) const {
		stream.writeUnsignedVarInt(3); // use item on entity
		stream.writeUnsignedVarInt(0); // action count
		stream.writeUnsignedVarInt64(entityID); // entity id
		stream.writeUnsignedVarInt(1); // attack entity (0 is interact)
		stream.writeVarInt(slot); // hotbar slot (0-8)
		serialize<ItemStack>::write(*Util::player->getInventoryMenu().getSlot(slot), stream);
		// below are little endian floats
		// player position (you)
		stream.writeFloat(Util::player->getPos().x);
		stream.writeFloat(Util::player->getPos().y);
		stream.writeFloat(Util::player->getPos().z);
		// click position (all 0s for this type)
		stream.writeFloat(0);
		stream.writeFloat(0);
		stream.writeFloat(0);
	};

	virtual void read(ReadOnlyBinaryStream &stream) {};

	//virtual bool disallowBatching() const { return false; }
};