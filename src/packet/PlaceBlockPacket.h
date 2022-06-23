#pragma once

#include "network/protocol/Packet.h"
#include "math/BlockPos.h"

struct PlaceBlockPacket : Packet {
	const BlockPos blockPos;
	const unsigned char face;
	short slot;

	PlaceBlockPacket(BlockPos blockPos, unsigned char face = 255, int slot = Util::player->getSelectedItemSlot()) : blockPos(blockPos), face(face), slot(slot) {}

	virtual int getId() const {
		return 0x1e;
	};

	virtual std::string getName() const {
		return "InventoryTransactionPacket";
	};

	virtual void write(BinaryStream &stream) const {
		ContainerItemStack *stack = Util::player->getInventoryMenu().getSlot(slot);
		int id = stack->getId();
		unsigned short aux = stack->getStackSize(); // blocks don't usually have meta values so imma just ignore that

		stream.writeUnsignedVarInt(2); // type use item
		stream.writeUnsignedVarInt(1); // action count actually has new item and old item with new decreased size after placing

		// begin action 1
		stream.writeUnsignedVarInt(0); // source type
		stream.writeVarInt(0); // window id
		stream.writeUnsignedVarInt(slot); // inventory slot
		serialize<ItemStack>::write(*stack, stream);
		stack->decreaseCount(1);
		serialize<ItemStack>::write(*stack, stream);
		// end action 1

		stream.writeUnsignedVarInt(0); // click block type
		stream.writeBlockPos(blockPos); // block
		stream.writeVarInt(face); // face
		stream.writeVarInt(slot); // slot
		// cause the old count is held
		stack->increaseCount(1);
		// heldItem
		serialize<ItemStack>::write(*stack, stream);
		// player position
		stream.writeVec3(Util::player->getPos());
		// click vec
		stream.writeVec3({0.3, 1, 0.3});

		stream.writeUnsignedVarInt(Util::player->getRegion().getBlock(blockPos).getRuntimeId()); // block runtime id
	};

	virtual void read(ReadOnlyBinaryStream &stream) {};

	//virtual bool disallowBatching() const { return false; };
};