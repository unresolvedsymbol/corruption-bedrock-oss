#pragma once

#include "../Module.h"
#include <map>

class Keybinds : public Module {
	std::unordered_multimap<char, std::string> bindings; // Key, Command
	int oldStates[256] {};

public:
	Keybinds();
};
