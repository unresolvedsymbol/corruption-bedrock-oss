#pragma once

#include "EventPriority.h"
#include <vector>
#include <algorithm>
#include <type_traits>

template<class T>
using Listener = std::function<void(T &)>;

using ListenerHandle = unsigned short;

/**
 * @tparam T
 * @author Void :^)
 *
 * Still fucking bugs sometimes for no reason
 */
struct EventChannel {
	static bool destructed;

	template<typename T>
	struct RegisterEntry {
		Listener<T> listener;
		EventPriority priority;
		ListenerHandle handle;

		RegisterEntry(Listener<T> listener, EventPriority priority, ListenerHandle handle) : listener(std::move(listener)), priority(priority), handle(handle) {};
	};

	// Using a unique pointer for registered and a raw pointer for increases performance but causes memory bugs
	// This currently handles about 60 million events per second (based on single firing, listener benchmarks)

	template<typename T>
	static auto &registered() {
		static std::vector<std::shared_ptr<RegisterEntry<T>>> cache{};
		return cache;
	};

	template<typename T>
	static auto &enabled() {
		static std::vector<std::shared_ptr<RegisterEntry<T>>> cache{};
		return cache;
	};

	template<typename T>
	static auto& cachedSize() {
		static ListenerHandle size = 0;
		return size;
	}

	template<typename T>
	static void sort() {
		std::sort(enabled<T>().begin(), enabled<T>().end(), [](auto a, auto b) {
			return a->priority > b->priority;
		});
	}

public:

	template<typename T>
	static void registerStaticListener(Listener<T> listener, EventPriority priority = EventPriority::Normal) {
		registerDynamicListener(listener, priority, true);
	}

	template<typename T>
	static auto registerDynamicListener(Listener<T> listener, EventPriority priority = EventPriority::Normal, bool enable = false) {
		static ListenerHandle handle = 0;
		handle++;
		registered<T>().push_back(std::make_shared<RegisterEntry<T>>(listener, priority, handle));
		if (enable)
			setEnabled<T>(handle, true);
		return handle;
	}

	// A penalty on listener enabling/disabling is much bett
	template<typename T>
	static void setEnabled(ListenerHandle handle, bool state) {
		if (state) {
			std::copy_if(registered<T>().begin(), registered<T>().end(), std::back_inserter(enabled<T>()), [=](auto l) { return l && l->handle == handle; });
			sort<T>();
		} else
			enabled<T>().erase(std::remove_if(enabled<T>().begin(), enabled<T>().end(), [=](auto l) { return l && l->handle == handle; }));
		cachedSize<T>() = enabled<T>().size();
	}

	template<typename T>
	static void remove(ListenerHandle handle) {
		setEnabled<T>(handle, false);
		registered<T>().erase(std::remove_if(registered<T>().begin(), registered<T>().end(), [=](auto l) { return l && l->handle == handle; }));
	}

	template<typename T>
	static T fire(T t) {
		static_assert(std::is_object<T>::value);

		if (destructed) {
			enabled<T>().clear();
			cachedSize<T>() = 0;
			return t;
		}

		// Ranged based, should work concurrently. Yields about 19000 ~events/ms
		ListenerHandle handle = 0;
		while (handle < cachedSize<T>())
			enabled<T>()[handle++]->listener(t);

		// For each. Yields about 10000 events/ms
		/*for (const auto &l : enabled<T>())
			l->listener(t);*/

		// Iterator + lambda based. Yields about 9000 events/ms
		/*std::for_each(enabled<T>().begin(), enabled<T>().end(), [&](const auto &l) {
			l->listener(t);
		});*/

		// Iterator based. Also yields about 9000 events/ms.
		/*auto it = enabled<T>().begin();
		while (it != enabled<T>().end())
			(*it++)->listener(t);*/

		return t;
	}
};
