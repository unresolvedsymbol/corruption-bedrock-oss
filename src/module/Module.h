#pragma once

#include <utility>
#include <string>
#include <vector>
#include <regex>
#include <iostream>
#include <optional>

#include "events/impl/PlayerEvents.h"

using args_t = std::vector<std::string> &;
using chat_t = std::optional<ChatEvent>;
using run_t = std::function<void(chat_t, args_t)>;

class Module {
	// TODO: Use a function container with less overhead
	run_t run;

public:
	const std::string name, strippedName, abstractedName;
	Module *parent = nullptr;

	std::vector<Module*> modules;

	explicit Module(std::string name, Module *parent = nullptr);

	Module *setRun(run_t run);

	void list();

	virtual void execute(chat_t chatEvent, args_t args);

	std::string getName() const;

	Module *get(std::string &alias);

	Module *addModule(Module *module);

	virtual bool isToggleable() const;

	virtual ~Module() = default;
};

