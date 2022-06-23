#pragma once

#include "Value.h"
#include <utility>
#include "util/Log.h"
#include "events/EventChannel.h"
#include "events/EventPriority.h"

class ToggleModule : public Value<bool> {
	std::vector<std::pair<ListenerHandle, void (*) (ListenerHandle, bool)>> listeners; // Stores listener handle and function pointer for it's enable function

protected:
	std::function<void()> onDisable, onEnable;

public:

	ToggleModule *setOnEnable(std::function<void()> onEnable);

	ToggleModule *setOnDisable(std::function<void()> onDisable);

	template<typename T>
	ToggleModule *addDynamicListener(Listener<T> listener, EventPriority priority = EventPriority::Normal);;

	explicit ToggleModule(const std::string& name, bool state = false);

	ToggleModule *set(bool) override;

	bool isToggleable() const override;

	~ToggleModule() override;
};

template<typename T>
ToggleModule *ToggleModule::addDynamicListener(Listener<T> listener, EventPriority priority) {
	listeners.emplace_back(EventChannel::registerDynamicListener<T>(listener, priority, get()), &EventChannel::setEnabled<T>);
	return this;
}
