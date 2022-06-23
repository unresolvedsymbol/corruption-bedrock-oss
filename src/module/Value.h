#pragma once

#include "Module.h"
#include "../util/Log.h"
#include <sstream>
#include <type_traits>

template<typename T>
class Value : public Module {
	T value;

protected:
	//T cached;

public:
	Value(Module *parent, const std::string &name, T init);

	virtual Value<T> *set(T value);

	T get();

	T operator*();

	virtual T operator=(T value);

	~Value();
};

template<typename T>
Value<T>::Value(Module *parent, const std::string &name, T init) : Module(name, parent), value(init) {
	addModule((new Module("Set"))->setRun([&](auto, args_t args) {
		T temp;
		if (Util::parse(args, 0, temp)) {
			set(temp);
			// if same bool si now??
			if constexpr (std::is_arithmetic<T>::value)
				Log() << "Set " << this->abstractedName << " to " << +value;
			else
				Log() << "Set " << this->abstractedName << " to " << value;
		} else
			Log() << "Invalid value.";
	}));
	setRun([&](auto, args_t) {
		if constexpr (std::is_same<T, bool>::value)
			Log() << "Switched " << this->abstractedName << " " << (set(!value)->value ? "on" : "off");
		else
		if constexpr (std::is_arithmetic<T>::value)
			Log() << this->abstractedName << " is set to " << +value;
		else
			Log() << this->abstractedName << " is set to " << value;
	});
}

template<typename T>
Value<T> *Value<T>::set(T value) {
	this->value = value;
	return this;
}

template<typename T>
T Value<T>::get() {
	return this->value;
}

template<typename T>
T Value<T>::operator*() {
	return value;
}

template<typename T>
T Value<T>::operator=(T value) {
	return set(value)->value;
}

template<typename T>
Value<T>::~Value() = default;