#pragma once

#include <fstream>
#include <endian.h>
#include <iostream>
#include <map>

// Reder for https://www.stuffbydavid.com/mcnbs/format by Void
class NBSReader {
	std::ifstream f;

	using s8 = int8_t;
	using s16 = int16_t;
	using s32 = int32_t;

	s8 rs8();
	s16 rs16();
	s32 rs32();

	std::string rstr();

public:
	s16 length, tempo;
	std::multimap<s16, std::pair<s16, s16>> notes;

	explicit NBSReader(std::string path);
};
