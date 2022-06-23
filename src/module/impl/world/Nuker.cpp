#include <world/VanillaBlocks.h>
#include "Nuker.h"
#include "events/impl/MiscEvents.h"
#include "world/BedrockBlocks.h"
#include <optional>
#include <packet/AnonymousPacket.h>
#include <util/serialize.h>

struct CakeBlock {
	bool use(Player&, BlockPos const&, unsigned char) const;
};
struct VanillaBlockTypes {
	static const Block *mCake;
	static const Block *mCarrotCrop;
};

Nuker::Nuker() : ToggleModule("BlockAura") {
	std::optional<Actor> xd;
	addDynamicListener<PlayerTick>([&](auto &) {
		if (Util::player->ticksExisted % 2 == 0) {
			int r = *range, c = *count;
			for (int y = r; y > -r; --y) {
				for (int x = -r; x < r; ++x) {
				for (int z = -r; z < r; ++z) {
						BlockPos blockPos = Util::player->getPos().add(x, y, z);
						Block &block = Util::player->getRegion().getBlock(blockPos);
						/*if (block != BedrockBlocks::mAir && (*bedrock || block != VanillaBlocks::mBedrock)) {
							for (unsigned char face = Direction::DOWN; face <= Direction::EAST; ++face) {
								if (Util::player->getRegion().getMaterial(pos.neighbor(face)).isReplaceable()) {
									Util::player->gameMode->destroyBlock(pos, face + face % 2 == 0 ? 1 : -1);
									break;
								}
							}
							if (!--c)
								goto exit;
						}*/
						if (block == VanillaBlocks::mCake || block == VanillaBlockTypes::mCake) {
							Log(1) << "cake";
							Util::sendPacket(AnonymousPacket{"InventoryTransactionPacket", 0x1e, [&](BinaryStream &stream) {
								stream.writeVarInt(0); // no changed item bs
								stream.writeUnsignedVarInt(2); // use item
								stream.writeBool(false); // has item stack ids

								stream.writeUnsignedVarInt(0); // action count (none)
								stream.writeUnsignedVarInt(0); // on block
								stream.writeBlockPos(blockPos); // at block
								stream.writeVarInt(Direction::UP); // face
								stream.writeVarInt(0);
								stream.writeVarInt(0); // empty item lol
								//serialize<ItemStack>::write(*Util::player->getInventoryMenu().getSlot(Util::player->getSelectedItemSlot()), stream); //iteminhand
								stream.writeVec3(Util::player->pos);
								// click position (all 0s for this type)
								stream.writeVec3({.5f / (Util::player->ticksExisted % 20), .5f, .5f / (Util::player->ticksExisted % 20)});
								// block runtime id
								stream.writeUnsignedVarInt(VanillaBlocks::mCake->getRuntimeId());
							}}, false);
							//goto exit;
						}
						if (block == VanillaBlocks::mCarrotCrop || block == VanillaBlockTypes::mCarrotCrop) {
							Log(1) << "carrots";
							Util::player->gameMode->destroyBlock(blockPos, Direction::UP);
							return;
							//goto exit;
						}
					}
				}
			}
			exit:
			return;
		}
	});
}
