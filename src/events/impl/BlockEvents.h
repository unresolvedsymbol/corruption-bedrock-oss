#pragma once

#include "util/Brightness.h"
#include "world/Block.h"

struct BlockColorEvent {
	const Block &block;
	unsigned int color;
};

struct BlockRenderLayerEvent {
	const Block &block;
	int value;
};

struct BlockLightEvent {
	const Block &block;
	Brightness value;
};

struct BlockTranslucentEvent {
	const Block &block;
	bool opaque;
};

struct EmptyBlockEvent {
	Block &block;
	bool empty;
};
