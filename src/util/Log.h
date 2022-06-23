#pragma once

#include "Util.h"
#include <ostream>
#include <fstream>
#include <string>
#include <utility>

struct Log : std::ostringstream {
	static std::ofstream logFile;
	static bool silence;
	bool silent;

	Log(bool silent = false);

	~Log() override;
};
