#include "Phase.h"

Phase::Phase() : ToggleModule("Phase") {
	addDynamicListener<MoveEvent>([&](auto &me) {
		if (Util::player->collidedHorizontally && (!*sneak || Util::player->isSneaking())) {
			auto move = Util::getMoveVec(*distance);
			Util::player->aabb.offset(move.x, *vertical, move.y);
		}
	});
}