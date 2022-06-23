#include "Hitboxes.h"

Hitboxes::Hitboxes() : ToggleModule("Hitboxes") {
	addDynamicListener<PlayerTick>([&](auto &) {
		if (Util::player->ticksExisted % 10 != 0)
			return;
		Util::player->getLevel().forEachPlayer([&](Player &p) {
			p.setSize(Util::valid(p, true) ? *size : .6f, 1.8f);
			return true;
		});
	});
	setOnDisable([&]() {
		Util::player->getLevel().forEachPlayer([](Player &p) {
			p.setSize(.6f, 1.8f);
			return true;
		});
	});
}
