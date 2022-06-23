#include "Corruption.h"
#include <client/input/Keyboard.h>
#include "Keybinds.h"
#include "events/impl/MiscEvents.h"

Keybinds::Keybinds() : Module("Bind"), bindings{
	// Default keybindings, doesn't save yet
	{'R', "aura"},
	{'F', "glide"},
	{'C', "speed"},
	{'K', "phase"},
	{'B', "blink"},
	{'V', "freecam"},
	{'Z', "zoom"},
	{'X', "wallhack"},
	{'N', "blockaura"}
} {
	setRun([](auto, args_t args) {
		Log() << "Usage: bind <add|delete> <key> [command]";
	});

	addModule((new Module("Delete"))->setRun([&](auto, args_t args) {
		char key;
		if (!Util::parse(args, 0, key)) {
			Log() << "Usage: bind delete <key>";
			return;
		}
		key = std::toupper(key);
		auto range = bindings.equal_range(key);
		if (range.first == range.second) {
			Log() << "\"" << key << "\" is not bound.";
			return;
		}
		for (auto bind = range.first; bind != range.second; ++bind)
			Log() << "Unbound \"" << key << "\" from \"" << bind->second << "\".";
		bindings.erase(key);
	}));

	addModule((new Module("Add"))->setRun([&](auto, args_t args) {
		char key;
		std::string command;
		if (!Util::parse(args, 0, key) || !Util::parseRest(args, 1, command)) {
			Log() << "Usage: bind add <key> <command>";
			return;
		}
		key = static_cast<char>(std::toupper(key));
		bindings.emplace(key, command);
		Log() << "Bound \"" << key << "\" to \"" << command << "\".";
	}));

	addModule((new Module("Clear"))->setRun([&](auto, args_t args) {
		int count = bindings.size();
		bindings.clear();
		Log() << "Cleared " << count << " bindings.";
	}));

	EventChannel::registerStaticListener<PlayerTick>([&](auto &) {
		// Check if in game && no window open
		if (Util::client->getClientSceneStack()->getSize() != 2)
			return;
		Log::silence = true;
		for (auto &bind : bindings)
			if (Keyboard::_states[bind.first] && !oldStates[bind.first])
				Corruption::get()->modules().handle(std::nullopt, {bind.second});
		Log::silence = false;
		for (const auto &bind : bindings)
			oldStates[bind.first] = Keyboard::_states[bind.first];
	});
}
