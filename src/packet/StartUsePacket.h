#pragma once

#include "util/Util.h"
#include "util/serialize.h"
#include "network/protocol/Packet.h"

struct StartUsePacket : Packet {
	const short slot;
	ItemStack *stack;

	StartUsePacket(short slot = Util::player->getSelectedItemSlot(), ItemStack *stack = nullptr) : slot(slot), stack(stack) {
		if (!stack)
			this->stack = Util::player->getInventoryMenu().getSlot(slot);
	}

	virtual int getId() const {
		return 0x1e;
	}

	virtual std::string getName() const {
		return "InventoryTransactionPacket";
	}

	virtual void write(BinaryStream &stream) const {
		stream.writeUnsignedVarInt(2); // use item
		stream.writeUnsignedVarInt(0); // action count
		stream.writeUnsignedVarInt(1); // click air
		// \/ valid for air
		stream.writeBlockPos(BlockPos::ZERO);
		stream.writeVarInt(Direction::INVALID); // face
		stream.writeVarInt(slot); // hotbar
		serialize<ItemStack>::write(*stack, stream);
		stream.writeVec3(Util::player->getPos());
		// click position (all 0s for this type)
		stream.writeVec3(Vec3::ZERO);
		stream.writeUnsignedVarInt(0); // block runtime id
	}

	virtual void read(ReadOnlyBinaryStream &stream) {};

	//virtual bool disallowBatching() const { return false; };
};