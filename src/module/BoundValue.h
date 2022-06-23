#pragma once

#include "Module.h"
#include "Value.h"

template<typename T>
struct DefaultComparator {
	constexpr int operator()(const T& x, const T& y) const {
		return x < y ? -1 : x == y ? 0 : 1;
	}
};

template<typename T, typename Compare = DefaultComparator<T>>
class BoundValue : public Value<T> {
	const T min, max;

public:
	BoundValue(Module *parent, const std::string& name, T init, T min, T max);

	Value<T> *set(T value) override;

	T operator=(T value) override;

	~BoundValue();
};

template<typename T, typename Compare>
BoundValue<T, Compare>::BoundValue(Module *parent, const std::string& name, T init, T min, T max) : Value<T>(parent, name, init), min(min), max(max) {}

template<typename T, typename Compare>
Value<T> *BoundValue<T, Compare>::set(T value) {
	const Compare compare;
	if (compare(value, min) < 0)
		return Value<T>::set(min);
	else if (compare(value, max) > 0)
		return Value<T>::set(max);
	else
		return Value<T>::set(value);
}

template<typename T, typename Compare>
T BoundValue<T, Compare>::operator=(T value) {
	return set(value)->get();
}

template<typename T, typename Compare>
BoundValue<T, Compare>::~BoundValue() = default;

//template<typename T, typename Compare>
//constexpr bool is_setting<BoundValue<T, Compare>> = true;