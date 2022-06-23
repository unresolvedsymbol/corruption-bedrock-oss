#include "NBSReader.h"

// I spent the good part of a day...
NBSReader::NBSReader(std::string path) {
	f.open(path, std::ios::binary | std::ios::in);
	if (!f.is_open())
		throw std::ifstream::failure{"File couldn't be opened."};
	// HEADER
	if ((length = rs16()) == 0) { // song length in ticks (0 if new nbs)
		s8 version = rs8(); // new nbs version
		rs8(); // vanjlla instrument count
		if (version == 3)
			length = rs16(); // song length
	}
	rs16(); // song height
	rstr(); // song name
	rstr(); // song author
	rstr(); // original song author
	rstr(); // song description
	tempo = rs16(); // tempo in ticks
	rs8(); // auto saving
	rs8(); // auto saving duration
	rs8(); // time sig
	rs32(); // minutes spent on proj
	rs32(); // left clicks
	rs32(); // right clicks
	rs32(); // blocks added
	rs32(); // blocks removed
	rstr(); // imported file name
	// NOTE BLOCKS
	s16 tick = -1;
	//for (s16 jumps = 0; jumps != 0; jumps = rs16())
	s16 jumps = 0;
	while (true) {
		jumps = rs16();
		if (jumps == 0)
			break;
		tick += jumps;
		s16 layer = -1;
		while (true) {
			jumps = rs16();
			if (jumps == 0)
				break;
			layer += jumps;
			s8 instrument = rs8();
			s8 key = rs8();
			if (key < 33 || key > 57)
				std::cerr << "Note block has invalid key?";
			notes.insert({tick * (20 / (tempo / 100)), {instrument, key - 33}});
		}
	}
	f.close();
}

NBSReader::s8 NBSReader::rs8() {
	s8 buf;
	f.read(reinterpret_cast<char *>(&buf), sizeof(s8));
	return buf;
}

NBSReader::s16 NBSReader::rs16() {
	s16 buf;
	f.read(reinterpret_cast<char *>(&buf), sizeof(s16));
	buf = le16toh(buf);
	return buf;
}

NBSReader::s32 NBSReader::rs32() {
	s32 buf;
	f.read(reinterpret_cast<char*>(&buf), sizeof(s32));
	buf = le32toh(buf);
	return buf;
}

std::string NBSReader::rstr() {
	s32 len = rs32();
	char buf[len];
	f.read(reinterpret_cast<char*>(&buf), len);
	buf[len] = 0; // add null terminator
	std::string str(buf, len);
	return str;
}
