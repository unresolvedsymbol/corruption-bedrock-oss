#include <network/protocol/LevelSoundEventPacket.h>
#include "SongStealer.h"
#include "events/impl/PacketEvents.h"
#include "Corruption.h"

// Not like anyone else has a notebot on bedrock to steal songs from tho... :(
SongStealer::SongStealer() : ToggleModule("SongStealer") {
	setOnEnable([&]() {
		notes.clear();
		delay = tick = 0;
		started = false;
	});
	addDynamicListener<PreReadEvent<LevelSoundEventPacket>>([&](auto &pe) {
		if (pe.packet.sound != 81) // Note
			return;
		int noteOrd = pe.packet.extra % 256;
		if (noteOrd > 21)
			Log() << "Invalid note " << tick << ":" << pe.packet.extra << "?";
		notes.emplace_back(tick, pe.packet.extra);
		if (!started)
			started = true;
		delay = 0;
	});
	addDynamicListener<PlayerTick>([&](auto &) {
		if (!started)
			return;
		tick++;
		if (++delay > 150) {
			Log() << "Assuming song ended. Saving.";
			set(false);
		}
	});
	setOnDisable([&]() {
		char buffer[46];
		time_t timeNow;
		time(&timeNow);
		strftime(buffer, 46, "/home/void/songs/stolen-%F-%X.txt", localtime(&timeNow));
		std::ofstream of{buffer};
		if (!of.is_open()) {
			Log() << "Couldn't open file for saving!";
			return;
		}
		for (auto note : notes) {
			int noteOrd = note.second % 256;
			of << note.first << ":" << noteOrd << ":" << ((note.second - noteOrd) / 256) << '\n';
		}
		of.close();
		Log() << "Saved " << notes.size() << " notes!";
	});
}