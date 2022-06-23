#include <network/protocol/LevelSoundEventPacket.h>
#include "NoteBot.h"
#include "util/NBSReader.h"

NoteBot::NoteBot() : ToggleModule("NoteBot") {
	auto convertInstrument = [](int instrument) {
		// Name		Bedrock	Java
		// Harp		0		0
		// Stone 	256		1
		// Snare 	512		2
		// Glass	768		3
		// Wood		1024	4
		// Use harp for unsupported primary instruments
		return instrument < 5 ? instrument * 256 : 0;
	};
	addModule((new Module("Load"))->setRun([&](auto, args_t args) {
		int format;
		std::string path;
		if (!Util::parse(args, 0, format) || !Util::parse(args, 1, path)) {
			Log() << "Usage: .notebot load [format] [song]";
			return;
		}
		path = getenv("HOME") + ("/songs/" + path);
		try {
			switch (format) {
				case 0: {
					NBSReader nbs{path};
					notes.clear();
					for (auto note : nbs.notes)
						notes.insert({note.first, convertInstrument(note.second.first) + note.second.second});
					end = nbs.notes.rbegin()->first;
					break;
				}
				case 1: {
					std::ifstream f{path};
					if (!f.is_open())
						throw std::ifstream::failure{""};
					int t, n, i;
					char c;
					notes.clear();
					while (f >> t >> c >> n >> c >> i && c == ':')
						notes.insert({t, n + convertInstrument(i)});
					end = t;
					f.close();
					break;
				}
			}
			tick = 0;
			Log() << "Loaded " << notes.size() << " notes.";
		} catch (...) {
			Log() << "Failed to read file.";
		}
	}));
	addModule((new Module("Convert"))->setRun([&](auto, args_t args) {
		std::string path;
		if (!Util::parse(args, 0, path)) {
			Log() << "Usage: .notebot convert [nbs file]";
			return;
		}
		try {
			NBSReader nbs{path};
			std::ofstream f{"/home/void/songs/converted.txt"};
			for (auto note : nbs.notes)
				f << (note.first * (nbs.tempo / 300)) << ":" << note.second.second << ":" << note.second.first << '\n';
			f.close();
			Log() << "Converted.";
		} catch (std::ifstream::failure f) {
			Log() << f.what();
		}
	}));
	setOnEnable([&]() {
		if (notes.empty())
			Log() << "No song loaded!";
		tick = 0;
	});
	addDynamicListener<PlayerTick>([&](auto &) {
		if (notes.empty())
			return;
		auto eq = notes.equal_range(tick);
		for (auto it = eq.first; it != eq.second; ++it)
			Util::sendPacket(LevelSoundEventPacket{81, Util::player->getPos().add(3, 2, 3), ":", true, it->second, false});
		if (++tick > end)
			tick = 0; // Loop
	});
}