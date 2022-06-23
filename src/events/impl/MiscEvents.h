#pragma once

#include "util/MapDecoration.h"
#include "events/Cancellable.h"
#include "client/gui/screens/models/ClientInstanceScreenModel.h"
#include "entity/Actor.h"

struct MapUpdateEvent {
	const std::vector<std::shared_ptr<MapDecoration>> &decorations;
	const std::vector<ActorUniqueID> &ids;
};

struct OSEvent {
	int type;
};


struct ItemHook {
	ItemStackBase &stack;
};
