#include "Module.h"
#include "util/Log.h"

Module::Module(std::string name, Module *parent) : name(std::move(name)), strippedName(this->name), parent(parent), abstractedName((parent ? parent->abstractedName + "::" : "") + this->name), run([&](auto, args_t) { list(); }) {
	auto &stripping = const_cast<std::string&>(strippedName);
	std::transform(stripping.begin(), stripping.end(), stripping.begin(), ::tolower);
	stripping.erase(std::remove_if(stripping.begin(), stripping.end(), ::isspace), stripping.end());
	if (parent)
		parent->addModule(this);
	else
		this->addModule((new Module("List", this))->setRun([&](chat_t, args_t) { list(); }));
}

void Module::list() {
	if (modules.empty()) {
		Log() << "§o" << this->getName() << "§r§c has no available submodules.";
		return;
	}
	std::string ss = "Submodules: ";
	for (auto mod : modules)
		ss += "§7" + mod->getName() + "§8, ";
	ss.erase(ss.length() - 2);
	Log() << "Available modules: " << ss;
}

Module *Module::setRun(run_t run) {
	this->run = std::move(run);
	return this;
}

Module *Module::addModule(Module *module) {
	modules.push_back(module);
	return module->parent = this;
}

Module *Module::get(std::string &alias) {
	alias.erase(std::remove(alias.begin(), alias.end(), ' '), alias.end());
	std::transform(alias.begin(), alias.end(), alias.begin(), ::tolower);
	Module *best = nullptr;
	unsigned char bestLen = 255;
	for (auto sub : modules) {
		if (sub->strippedName.find(alias, 0) == 0) {
			unsigned char len = sub->strippedName.length() - alias.length();
			if (len <= bestLen) {
				best = sub;
				bestLen = len;
			}
		}
	}
	return best;
}

std::string Module::getName() const {
	return name;
}

void Module::execute(chat_t chatEvent, args_t args) {
	// Sub module commands
	if (!args.empty()) {
		if (Module *mod = get(args.front())) {
			args.erase(args.begin()); // Remove this argument and pass onto sub module
			mod->execute(chatEvent, args);
			return;
		}
	}

	// Couldn't find submodule, just pass to it's run function; it's probably parsing the arguments
	run(chatEvent, args);
}

bool Module::isToggleable() const {
	return false;
}