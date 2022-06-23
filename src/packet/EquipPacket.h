#pragma once

#include <util/serialize.h>

struct EquipPacket : Packet {
	short slot;
	const ContainerItemStack *stack;

	EquipPacket(short slot, const ContainerItemStack *stack = nullptr) : slot(slot), stack(stack) {
		if (stack == nullptr) this->stack = Util::player->getInventoryMenu().getSlot(slot);
	}

	virtual int getId() const {
		return 31;
	};

	virtual std::string getName() const {
		return "MobEquipmentPacket";
	};

	virtual void write(BinaryStream &stream) const {
		stream.writeUnsignedVarInt64(Util::player->getRuntimeID());
		serialize<ItemStack>::write(*stack, stream);
		stream.writeByte(slot); // inv slot
		stream.writeByte(slot); // hotbar slot
		stream.writeByte(0); // window id
	}

	virtual void read(ReadOnlyBinaryStream &stream) {};

	//virtual bool disallowBatching() const { return false; };
};