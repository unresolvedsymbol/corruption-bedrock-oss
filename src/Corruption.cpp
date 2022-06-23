#include <network/protocol/PlaySoundPacket.h>
#include "Corruption.h"
#include "events/impl/PacketEvents.h"
#include "network/protocol/ActorEventPacket.h"

#if defined(__arm__)
#include "util/JNI.h"
#include "util/Substrate.h"
#endif

bool Log::silence = false;
std::ofstream Log::logFile{
#if defined(__arm__)
	"/sdcard"
#else
	std::string(std::getenv("HOME")) +
#endif
	"/corruption.log"
};
bool EventChannel::destructed = false;

Corruption *Corruption::instance = new Corruption;

Corruption *Corruption::get() {
	return instance;
}

Corruption::Corruption() {
	// Don't notify server about ability changes
	EventChannel::registerStaticListener<PreSendEvent<Packet>>([](auto &pe) { if (pe.packet.getId() == 0x37) pe.cancel(); });

	// Color codes.
	EventChannel::registerStaticListener<ChatEvent>([](auto &ce) {
		static const std::regex x("#");
		ce.text = std::regex_replace(ce.text, x, "\u00A7");
	}, EventPriority::Highest);

	/*EventChannel::registerStaticListener<PreReadEvent<PlaySoundPacket>>([](auto &pe) {
		Log() << "sound " << gsl::to_cstring(pe.packet.soundName) << " at " << pe.packet.pos;
	});*/

	// TODO: Move to module: ReachAlert or AntiCheat idk
	// Swing happens after the attack in vanilla clients
	/*EventChannel::registerStaticListener<PreReadEvent<AnimatePacket>>([](auto &pe) {
		if (!Util::client->isPreGame() && pe.packet.rid != Util::player->getRuntimeID() && pe.packet.action == AnimatePacket::Action::SwingArm && Util::player->hurtTime > 0) { // Self hurt animation
			if (auto *player = Util::player->getLevel().getRuntimePlayer(pe.packet.rid)) {
				float distance = Util::player->distanceTo(player->pos.add(player->pos.distance(player->prevPos)));
				if (distance < 6)
					Log() << player->getName() << " hit you from " << distance;
			}
		}
	});*/

	// TODO: Sort players by range internally when added if the type is sortable (I think it's some custom SmallSet or something?)

	Log(1) << "Corruption initialized";
}

ModuleManagerImpl_t &Corruption::modules() {
	return this->moduleManager;
}

Corruption::~Corruption() {
	//ImGui_ImplOpenGL3_Shutdown();
	Log::logFile.close();
}

extern "C" {
	//void* mcpelauncher_hook(void *sym, void *hook, void *orig) {}
	// Dummy MCPELauncher implementation (these are usuaully installed with hybris)
	void* mcpelauncher_hook2(void *lib, void *sym, void *hook, void *orig) {
#if defined(__arm__)
		// Fallback for Android
		MSHookFunction(symbol, hook, original);
#endif
		return nullptr;
	}

	void mcpelauncher_hook2_add_library(void *lib) {}
	void mcpelauncher_hook2_remove_library(void *lib) {}
	void mcpelauncher_hook2_delete(void *lib) {}
	void mcpelauncher_hook2_apply() {}
}

#if defined(__arm__)
JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
	//Keyboard::_states = new int[256] {}; // ok
	//MSHookFunction(dlsym(MinecraftHandle(), "_ZN6Common23getGameVersionStringNetEv"), (void**)&hooked, (void**)&og);
	return JNI_VERSION_1_6;
}
#endif