#pragma once

#include "../../ToggleModule.h"
#include "../../Value.h"

struct ESP : public ToggleModule {
	ESP();

	Value<bool> chams{this, "Chams", true};

private:
	std::unordered_map<unsigned int, std::vector<int>> potionCache;

	struct PlayerData {
		int health = 20;
		short air = 300;
		std::vector<MobEffect*> effects;
	};

	std::unordered_map<uint64_t, PlayerData> players;

	Value<bool> tags{this, "Name", true},
		health{this, "Health", true},
		box{this, "Box", true},
		armor{this, "Armor", true},
		effects{this, "Effects", true},
		tracers{this, "Tracers", true};
};
