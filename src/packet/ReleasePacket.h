#pragma once

#include "network/protocol/Packet.h"

struct ReleasePacket : Packet {
	bool bow;

	ReleasePacket(bool bow = false) : bow(bow) {};

	virtual int getId() const {
		return 0x1e;
	};

	virtual std::string getName() const {
		return "InventoryTransactionPacket";
	};

	virtual void write(BinaryStream &stream) const {
		unsigned short held = Util::player->getSelectedItem().getId();

		stream.writeUnsignedVarInt(4); // release item

		stream.writeUnsignedVarInt(0); // action count actually has new item and old item with new decreased size after eating

		stream.writeUnsignedVarInt(held == 261 ? 0 : 1); // bow = 0, food / potion = 1
		// \/ valid for air
		stream.writeVarInt(0); // block x
		stream.writeUnsignedVarInt(0); // block y
		stream.writeVarInt(0); // block z
		stream.writeVarInt(255); // face

		stream.writeVarInt(Util::player->getSelectedItemSlot()); // hotbar

		if (held) {
			stream.writeVarInt(held); // item id
			stream.writeVarInt(1); // damage
			stream.writeSignedShort(0); // nbt
			stream.writeVarInt(0); // can place on
			stream.writeVarInt(0); // can destroy
		} else {
			stream.writeVarInt(0);
		}
		stream.writeFloat(Util::player->getPos().x);
		stream.writeFloat(Util::player->getPos().y);
		stream.writeFloat(Util::player->getPos().z);

		// click position (all 0s for this type)
		stream.writeFloat(0);
		stream.writeFloat(0);
		stream.writeFloat(0);

		stream.writeUnsignedVarInt(0); // block runtime id
	};

	virtual void read(ReadOnlyBinaryStream &stream) {};

	//virtual bool disallowBatching() const { return false; };
};