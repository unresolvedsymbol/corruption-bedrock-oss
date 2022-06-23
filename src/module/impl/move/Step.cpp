#include <network/protocol/MovePlayerPacket.h>
#include <network/protocol/PlayerActionPacket.h>
#include "Step.h"
#include "events/impl/MiscEvents.h"

Step::Step() : ToggleModule("Step") {
	setOnDisable([]() {
		Util::player->stepHeight = .5625f;
	});
	addDynamicListener<MoveEvent>([&](auto &me) {
		// minus 1.8 from y?
		Vec3 pos{Util::player->getPos().x, Util::player->aabb.min.y, Util::player->getPos().z};
		auto &src = Util::player->getRegion(); // oof

		if (*vanilla) {
			Util::player->stepHeight = *maxHeight;

			// apparently min Y doesn't work anymore and lifeboat wants normal pos ??????
			/*auto posY = Util::player->getPos().y; //Util::player->aabb.min.y;

			auto diff = Util::player->aabb.min.y - lastMinY;
			if (Util::player->prevOnGround && diff > .5625f) {
				static const float offsets[] = {.45f, .72f, 1.01f};
				for (int i = 0; i < round(diff); ++i) {
					for (auto offset : offsets)
						Util::sendPacket(MovePlayerPacket{Util::player->getRuntimeID(), false,{Util::player->getPos().x, lastMinY + offset, Util::player->getPos().z},Vec3{Util::player->getRotation()}}, true);
					lastMinY += 1.01f;
				}
			}

			lastMinY = Util::player->aabb.min.y;*/
		} else {
			Util::player->stepHeight = .5625f;

			if (Util::player->onGround && (Util::player->collidedHorizontally/* || !src.getMaterial(pos.add(move.x, 0, move.y)).isReplaceable()*/) && (Util::input->forward || Util::input->strafe)) {
				auto move = Util::getMoveVec(.45f);

				static const float offsets[] = {.42f, .27f, .33f};
				// .42 .33
				// .45 .27 .29
				for (int height = 1; height <= *maxHeight; ++height) {
					// Don't step if not clear above head
					if (!src.getMaterial(pos.add(0, height + 1, 0)).isReplaceable())
						break;

					if (!src.getMaterial(pos.add(move.x, height - 1, move.y)).isReplaceable() &&
						// Check for player space atop block
						src.getMaterial(pos.add(move.x, height, move.y)).isReplaceable() &&
						src.getMaterial(pos.add(move.x, height + 1, move.y)).isReplaceable()) {
						if (*packets) {
							Util::sendPacket(PlayerActionPacket{Util::player->getRuntimeID(), PlayerActionPacket::Action::Jump},true);
							MovePlayerPacket pk{Util::player->getRuntimeID(), false, Util::player->getPos(),
												  Vec3{Util::player->getRotation()}};
							for (int x = 0; x < height; ++x) {
								for (auto offset : offsets) {
									pk.y += offset;
									Util::sendPacket(pk, true);
								}
							}
						}
						Util::player->aabb.offset(move.x, height + .01f, move.y);
						break;
					}
				}
			}
		}

		if (*reverse && !Util::player->onGround && Util::player->prevOnGround && me.motion.y < 0 &&
			src.getMaterial(pos.subtract(0, .001f, 0)).isReplaceable()) {
			for (int i = 0; i < *reverseHeight; ++i)
				if (!src.getMaterial(pos.subtract(0, i, 0)).isReplaceable()) {
					me.motion.y = -3;
					break;
				}
		}
	});
}
