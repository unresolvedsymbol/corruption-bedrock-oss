#include "ToggleModule.h"
#include <utility>

ToggleModule::ToggleModule(const std::string& name, bool state) : Value<bool>(nullptr, name, state) {}

ToggleModule *ToggleModule::setOnEnable(std::function<void()> onEnable) {
	this->onEnable = std::move(onEnable);
	return this;
}

ToggleModule *ToggleModule::setOnDisable(std::function<void()> onDisable) {
	this->onDisable = std::move(onDisable);
	return this;
}

ToggleModule *ToggleModule::set(bool value) {
	if (value != get()) {
		if (auto fn = value ? onEnable : onDisable)
			fn();
		for (auto pair : listeners)
			pair.second(pair.first, value);
		Value::set(value);
	}
	return this;
}

bool ToggleModule::isToggleable() const {
	return true;
}

ToggleModule::~ToggleModule() = default;