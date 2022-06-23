#include "Commands.h"

#include <network/protocol/TextPacket.h>
#include <util/MCPELauncherAPI.h>
#include <util/StaticHook.h>
#include <packet/StartUsePacket.h>
#include <item/EnchantUtils.h>
#include <Corruption.h>
#include "util/Log.h"
#include "packet/AnonymousPacket.h"
#include "events/EventChannel.h"
#include "dlfcn.h"
#include "Corruption.h"

List::List() : Module("List") {
	setRun([](auto, args_t) {
		std::string css, tss;
		for (auto *m : Corruption::get()->modules()) {
			if (m->isToggleable())
				tss += (dynamic_cast<ToggleModule *>(m)->get() ? "§a" : "§c") + m->getName() + "§7, ";
			else
				css += m->getName() + ", ";
		}
		tss.erase(tss.length() - 2);
		css.erase(css.length() - 2);
		Log() << "Toggleable: " << tss;
		Log() << "Runnable: " << css;
	});
}

Unload::Unload() : Module("Unload") {
	setRun([](chat_t chat, args_t) {
		if (!chat) {
			Log() << "Please run from chat";
			return;
		}

		// Begin unregistering any events that get fired
		EventChannel::destructed = true;

		// Disable modules for (hopefully) elegant cleanup
		for (auto *m : Corruption::get()->modules())
			if (m->isToggleable())
				dynamic_cast<ToggleModule *>(m)->set(false);

		// Delete messages
		// TODO: Only corruptions?
		//chat->screen.getGuiMessageList().clear();

		// Unhook (currently not impled in mcpelauncher, planning on doing it)
		for (; hookCache[hookHandle]; --hookHandle) {
			mcpelauncher_hook2_delete(hookCache[hookHandle]);
			hookCache[hookHandle] = nullptr;
		}
		mcpelauncher_hook2_apply();

		// Get lib info for deletion
		Dl_info info;
		dladdr(reinterpret_cast<void *>(&Corruption::get), &info);

		char libfile[256] {0};
		if (strrchr(info.dli_fname, '/')) {
			strncpy(libfile, info.dli_fname, 256);
		} else {
			FILE *fp = fopen("/proc/self/maps", "r");
			if (fp != nullptr) {
				// This is sometimes necessary on Android...
				char buffer[256] {0}, path[256] {0};
				while (fgets(buffer, 256, fp)) {
					if (sscanf(buffer, "%*llx-%*llx %*s %*s %*s %*s %s", path) == 1) {
						size_t pathSize = strlen(path), baseSize = strlen(info.dli_fname);
						for (size_t i = 0; i < baseSize; ++i)
							if (path[pathSize - i] != info.dli_fname[baseSize - i])
								goto next;
						strncpy(libfile, path, 256);
						break;
						next:;
					}
				}
				fclose(fp);
			}
		}

		// Destroy corruption instance
		delete Corruption::get();

		// TODO: Sanitize freed memory. memset 0 sizeof(Corruption) doesn't seem to work?
		mcpelauncher_hook2_remove_library(info.dli_fbase); // Doesn't work since unhooking doesn't (still wanted)

		// Delete
		// TODO: Preserve timestamps
		if (libfile[0])
			std::remove(libfile);

		// TODO: Poison mem
	});
}

Say::Say() : Module("Say") {
	setRun([](auto, args_t  args) {
		std::string msg;
		if (Util::parseRest(args, 0, msg))
			Util::sendPacket(TextPacket{TextPacketType::CHAT, "", msg, {}, false, "", ""});
		else
			Log() << "Usage: .say <message>";
	});
}

VClip::VClip() : Module("VClip") {
	setRun([](auto, args_t args) {
		float dist;
		if (!Util::parse(args, 0, dist)) {
			Log() << "Usage: .vclip <distance>";
			return;
		}
		Util::player->aabb.offset(0, dist, 0);
	});
}

HClip::HClip() : Module("HClip") {
	setRun([](auto, args_t args) {
		float dist, dir = Util::player->getRotation().yaw;
		if (!Util::parse(args, 0, dist)) {
			Log() << "Usage: .hclip <distance> [direction (deg)]";
			return;
		}
		Util::parse(args, 1, dir);
		auto move = Util::getMoveVec(dist, false);
		Util::player->aabb.offset(move.x, .05, move.y);
	});
}

Pot::Pot() : Module("Pot") {
	setRun([](auto, args_t args) {
		for (short slot = 0; slot < 9; ++slot) {
			auto stack = Util::player->getInventoryMenu().getSlot(slot);
			if (stack->isNull() ||
				stack->getId() != 438 || // Potion
				(stack->getAuxValue() != 21 && stack->getAuxValue() != 22 && stack->getAuxValue() > 30 && stack->getAuxValue() < 28)) // Instant health splashes (end is regeneration for some DUMB AS FUCK servers)
				continue;

			ListenerHandle handle = EventChannel::registerDynamicListener<PlayerTick>([=, &handle](auto &) {
				static unsigned char delay = 0, stage = 0, old = 0;
				if (delay > 0)
					delay--;
				else {
					switch (stage++) {
						case 0:
							old = Util::player->getSelectedItemSlot();
							Util::player->setSelectedItemSlot(slot);
							delay = 2;
							break;
						case 1:
							Util::sendPacket(StartUsePacket{slot, stack});
							delay = 1;
							break;
						case 2:
							stage = 0;
							delay = 2;
							Util::player->setSelectedItemSlot(old);
							EventChannel::remove<PlayerTick>(handle);
							break;
					}
				}
			}, EventPriority::Normal, true);

			break;
		}
	});
}

SetTimer::SetTimer() : Module("Timer") {
	setRun([](auto, args_t args) {
		float scale;
		if (Util::parse(args, 0, scale)) {
			Util::mc->getTimer()->setTimeScale(scale);
			Log() << "Timer scale set to " << scale;
		} else {
			Log() << "Timer scale is currently " << Util::mc->getTimer()->getTimeScale();
			Log() << "[ptrs] getter: " << std::hex << Util::mc->getTimer() << " field: " << std::hex << Util::mc->gameTimer << " render: " << std::hex << Util::mc->renderTimer;
		}
	});
}

AddEnchant::AddEnchant() : Module("Enchant") {
	setRun([](auto, args_t args) {
		int type = 0, level = 32767;
		if (!Util::parse(args, 0, type)) {
			Log() << "Usage: .enchant <id> [level = max]";
			return;
		}
		Util::parse(args, 1, level);
		auto *stack = Util::player->getInventoryMenu().getSlot(Util::player->getSelectedItemSlot());
		EnchantUtils::applyEnchant(*stack, static_cast<Enchant::Type>(type), level, true);
	});
}

EnchantSeed::EnchantSeed() : Module("EnchantSeed") {
	setRun([](auto, args_t args) {
		if (!args.size()) {
			Log() << "Current seed: " << std::hex << Util::player->getEnchantmentSeed();
			Log() << "Usage: .enchantseed <seed>";
			return;
		}
		std::stringstream ss;
		ss << std::hex << args[0];
		int seed;
		ss >> seed;
		Util::player->setEnchantmentSeed(seed);
		Log() << "Enchant seed set.";
	});
}

Effect::Effect() : Module("Effect") {
	setRun([](auto, args_t args) {
		unsigned int id;
		std::string input;

		if (!Util::parse(args, 0, id) && !Util::parse(args, 0, input)) {
			Log() << "Usage: .effect <effect | 0 = clear all> <duration = max | 0 = clear> [amplifier = 0]";
			return;
		}

		MobEffect *effect = MobEffect::getById(id);
		if (!effect)
			effect = MobEffect::getByName(input);

		if (!effect || effect == MobEffect::EMPTY_EFFECT) {
			if (!id) {
				Util::player->removeAllEffects();
				Log() << "All effects cleared";
			} else
				Log() << "Invalid effect ID or name provided";
			return;
		}

		id = effect->getId();
		auto name = effect->getResourceName();

		// YIKES
		char p = ' ';
		std::for_each(name.begin(), name.end(), [&](char &c) {
			if (p == ' ' && ::isalpha(c)) c = std::toupper(c); // Uppercase words
			if (c == '_') c = ' '; // Replace underscores with spaces
			p = c;
		});

		int duration = INT_MAX / 20;
		Util::parse(args, 1, duration);

		if (!duration) {
			Util::player->removeEffect(id);
			Log() << "Cleared " << name;
			return;
		}

		int amplifier = 0;
		Util::parse(args, 2, amplifier);

		Log() << name << " " << amplifier+1 << " added for " << duration << " seconds";

		Util::player->addEffect(MobEffectInstance{id, duration * 20, amplifier, 0, 0, false});
	});
}