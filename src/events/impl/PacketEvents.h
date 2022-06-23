#pragma once

#include "../Cancellable.h"

template<typename T>
struct PreSendEvent : Cancellable {
	T &packet;
	PreSendEvent(T &packet) : packet(packet) {}
};

template<typename T>
struct PreReadEvent : Cancellable {
	T &packet;
	PreReadEvent(T &packet) : packet(packet) {}
};

template<typename T>
struct PostSendEvent {
	const T &packet;
	PostSendEvent(const T &packet) : packet(packet) {}
	PostSendEvent(T &packet) : packet(const_cast<const T &>(packet)) {}
	operator bool() { return true; }
};

template<typename T>
struct PostReadEvent {
	const T &packet;
	PostReadEvent(const T &packet) : packet(packet) {}
	PostReadEvent(T &packet) : packet(const_cast<const T &>(packet)) {}
	operator bool() { return true; }
};
