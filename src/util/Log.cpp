#include "Log.h"

Log::~Log() {
	const auto stripped = Util::stripColors(this->str());
	std::cerr << "[Corruption] " + stripped << std::endl;
	// idk why the file broke
	Log::logFile << stripped << std::endl;
	if (!silent && Util::client && Util::client->isPlaying())
		Util::player->displayClientMessage("§8[§7C§8] §7" + this->str());
}

Log::Log(bool silent) : silent(silent), std::ostringstream() {}
