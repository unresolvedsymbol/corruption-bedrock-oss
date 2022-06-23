#pragma once

#include "../../ToggleModule.h"
#include "../../Value.h"
#include <map>

// TODO: BIG TODO: TAB UI YA LAZY FUCK (ME)

struct ArrayListData {
	float x, y,
		rX, rY,
		reX, reY;
	unsigned int color;
	const char *text;
};

struct Sorter {
	bool operator()(ArrayListData a, ArrayListData b) {
		return a.x > b.x;
	}
};

struct HUD : ToggleModule {
	HUD();

	Value<unsigned char> speed{this, "Speed", 1},
		moduleSpeed{this, "Mod Speed", 5},
		saturation{this, "Saturation", 120},
		value{this, "Value", 255};


	unsigned char hue;
	std::vector<ArrayListData> arrayList;
};
