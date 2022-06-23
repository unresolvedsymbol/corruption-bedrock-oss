#pragma once

#include "../ToggleModule.h"
#include "../Value.h"
#include <unordered_set>

class AntiCrash : public ToggleModule {
	struct SavedItem {
		short item;
		Vec3 pos;
	};

	Value<int> maxItems{this, "Max Items", 512};
	std::unordered_map<int64_t, SavedItem> items{};
	//std::unordered_set<int64_t> items;
	std::mutex itemsMutex;

public:
	AntiCrash();
};
