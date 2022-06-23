#pragma once

#include "network/BinaryStream.h"
#include "network/protocol/Packet.h"
#include <functional>

struct AnonymousPacket : Packet {
	const std::string name;
	const int id;
	// TODO: Template lambdas
	const std::function<void(BinaryStream &)> writeFunc;

	AnonymousPacket(std::string name, int id, std::function<void(BinaryStream &)> writeFunc) : name(name), id(id), writeFunc(writeFunc) {};

	virtual int getId() const {
		return id;
	}

	virtual std::string getName() const {
		return name;
	}

	virtual void write(BinaryStream &stream) const {
		writeFunc(stream);
	}

	virtual void read(ReadOnlyBinaryStream &stream) {}

	virtual bool disallowBatchinng() const { return false; };
};