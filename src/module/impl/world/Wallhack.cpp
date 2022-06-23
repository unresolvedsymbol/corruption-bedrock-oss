#include "Wallhack.h"
#include "events/impl/BlockEvents.h"

Wallhack::Wallhack() : ToggleModule("Wallhack") {
	addDynamicListener<BlockTranslucentEvent>([&](auto &e) {
		e.opaque = !wants(e.block);
	})->addDynamicListener<BlockColorEvent>([&](auto &e) {
		if (!wants(e.block)) {
			e.color &= 0x00FFFFFFu;
			e.color |= static_cast<unsigned int>(static_cast<unsigned char>(255u * *opacity) << 24u);
		}
	})->addDynamicListener<BlockLightEvent>([&](auto &e) {
		if (wants(e.block))
			e.value = static_cast<Brightness>(255u * *brightness);
	})->addDynamicListener<BlockRenderLayerEvent>([&](auto &e) {
		if (!wants(e.block))
			e.value = *opacity > 0 ? 1 : 9;
	})->setOnDisable([]() {
		Util::client->getBlockTessellator().clearBlockCache();
		Util::client->getLevelRenderer()->onAppSuspended();
		Util::client->getLevelRenderer()->onAppResumed();
	})->setOnEnable([]() {
		Util::client->getBlockTessellator().clearBlockCache();
		Util::client->getLevelRenderer()->onAppSuspended();
		Util::client->getLevelRenderer()->onAppResumed();
	});
}

bool Wallhack::wants(const Block &block) {
	// How'd I forget the actual ores and why'd I wrap this....
	static const std::unordered_set<uint64_t> cachedBlocks{
			VanillaBlocks::mIronOre->getRuntimeId().data,
			VanillaBlocks::mGoldOre->getRuntimeId().data,
			VanillaBlocks::mDiamondOre->getRuntimeId().data,
			VanillaBlocks::mLapisOre->getRuntimeId().data,
			VanillaBlocks::mQuartzOre->getRuntimeId().data,
			VanillaBlocks::mGlowStone->getRuntimeId().data,
			VanillaBlocks::mCoalOre->getRuntimeId().data,
			VanillaBlocks::mEmeraldOre->getRuntimeId().data,
			VanillaBlocks::mRedStoneOre->getRuntimeId().data,
			VanillaBlocks::mLitRedStoneOre->getRuntimeId().data,
			VanillaBlocks::mEnchantingTable->getRuntimeId().data,
			VanillaBlocks::mAnvil->getRuntimeId().data,
			VanillaBlocks::mQuartzOre->getRuntimeId().data,
			VanillaBlocks::mFlowingLava->getRuntimeId().data,
			VanillaBlocks::mStillLava->getRuntimeId().data,
			VanillaBlocks::mFlowingWater->getRuntimeId().data,
			VanillaBlocks::mStillWater->getRuntimeId().data,
			VanillaBlocks::mObsidian->getRuntimeId().data,
			VanillaBlocks::mGlowingObsidian->getRuntimeId().data,
			VanillaBlocks::mEndPortalFrame->getRuntimeId().data
	};
	return cachedBlocks.find(block.getRuntimeId().data) != cachedBlocks.end();
}
