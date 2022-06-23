#include <network/protocol/InteractPacket.h>
#include <network/protocol/AnimatePacket.h>
#include <network/protocol/PlayerActionPacket.h>
#include "Aura.h"
#include "packet/EquipPacket.h"
#include "packet/AnonymousPacket.h"
#include "events/impl/PacketEvents.h"
#include "events/impl/PlayerEvents.h"

Aura::Aura() : ToggleModule("Aura") {
	addModule((new Module("Preset"))->addModule(((new Module("Vanilla"))->setRun([&](auto, args_t) {
		multi = true;
		swing = false;
		slow = false;
		blockRaytrace = false;
		inventory = true;
		teleport = false;
		silentAim = false;
		range = 6;
		fov = 360;
		speed = 20;
		targets = 20;
		Log() << "Loaded Vanilla Aura";
	})))->addModule((new Module("Ghost"))->setRun([&](auto, args_t) { // A somewhat legitimate looking triggerbotish experience for ghosting, todo: bad swings
		inventory = false;
		silentAim = false;
		teleport = false;
		multi = false;
		swing = true;
		slow = true;
		blockRaytrace = true;
		range = 3.1;
		fov = 18;
		swingRange = 4;
		swingFov = 160;
		speed = 7;
		swingSpeed = 4;
		targets = 1;
		Log() << "Loaded Ghost Aura";
	}))->addModule((new Module("TP"))->setRun([&](auto, args_t) {
		inventory = false;
		silentAim = false;
		multi = false;
		swing = false;
		slow = false;
		blockRaytrace = false;
		teleport = true;
		teleportReturn = false;
		teleportRelative = -4;
		teleportRange = 5;
		teleportSpeed = 3;
		multi = true;
		swing = false;
		blockRaytrace = true;
		range = 30;
		swingFov = swingFov =  fov = 360;
		speed = 10;
		targets = 3;
		Log() << "Loaded TP Aura";
	}))->addModule((new Module("Hive"))->setRun([&](auto, args_t) {
		inventory = false;
		silentAim = true;
		multi = true;
		swing = true;
		blockRaytrace = true;
		teleport = false;
		range = 3.34;
		slow = true;
		swingFov = fov = 360;
		speed = 12;
		targets = 1;
		unsprint = false;
		Log() << "Loaded Hive Aura";
	}))->addModule((new Module("CubeCraft"))->setRun([&](auto, args_t) {
		inventory = false;
		silentAim = false; // unneeded if sending android/ios platform code on login
		multi = true;
		swing = true;
		blockRaytrace = true;
		teleport = false;
		range = 5;
		fov = 160;
		swingFov = 180;
		speed = 10;
		targets = 5;
		Log() << "Loaded CubeCraft Aura";
	})));

	// AntiBot memes
	EventChannel::registerStaticListener<PreReadEvent<AnimatePacket>>([&](auto &pe) {
		if (pe.packet.action == AnimatePacket::Action::SwingArm)
			playersSwung.emplace(pe.packet.rid);
	});

	EventChannel::registerStaticListener<PreReadEvent<Packet>>([&](auto &pe) {
		// Temporary antibot reset after world change on server. Not perfect. Map PlayerListPacket later then remove on list removall
		// AdventureSettingsPacket
		if (pe.packet.getId() == 0x37)
			playersSwung.clear();
	});

	EventChannel::registerStaticListener<PreSendEvent<Packet>>([&](auto &pe) {
		if (pe.packet.getId() == 0x1) // Clear bots on server change (LoginPacket)
			playersSwung.clear();
	});

	addDynamicListener<PlayerTick>([&](auto) {
		// Damageable is pretty close
		if ((!*inventory && !Util::client->isInGame()) ||
			(!*autoTool && *requireTool && !Util::player->getSelectedItem().isDamageableItem()))
			return;

		auto curTime = std::chrono::system_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(curTime - lastTime).count() > nextDelay) {
			int targetCount = 0;
			short oldHeld = -1;
			Vec3 sPos = Util::player->getPos();
			bool teleported = false;

			if (!Util::forEachPlayerSortedIf([&](Player &player) -> bool {
				// TODO Entity "raytrace" or just loop through entities and find invisibles within 3 blocks
				// oh wait kohi doesn't exist on bedrock yet

				if (*autoTool && oldHeld == -1 && !Util::player->getSelectedItem().isDamageableItem()) {
					for (short i = 0; i < 9; ++i) {
						auto *item = Util::player->getInventoryMenu().getSlot(i);
						if (!item || item->isNull())
							continue;
						short id = item->getId();
						if ((id > 255 && id < 259) || (id > 266 && id < 280)) { // Tool Item ID ranges (skip some random ones between iron tools and the rest)
							oldHeld = Util::player->getSelectedItemSlot();
							Util::player->setSelectedItemSlot(i);
							break;
						}
					}
					if (oldHeld == -1) // Didn't find tool. Abort.
						return false;
				}

				if (*teleport) {
					Vec3 pPos = player.getPos();

					if (auto rel = *teleportRelative) {
						auto move = Util::getMoveVec(rel, false, player.getRotation().yaw); // relative to targets look
						pPos.x += move.x;
						pPos.y += 3;
						pPos.z += move.y;
					}

					sPos = Util::stepTeleport(sPos, pPos, {*teleportSpeed, .4}, *teleportRange);

					teleported = true;
				}

				if (*silentAim)
					Util::sendPacket(MovePlayerPacket{Util::player->getRuntimeID(), Util::player->onGround, Util::player->getPos(), Vec3{Util::getRotationsTo(player)}}, true);

				bool resprint;
				if ((resprint = *unsprint && Util::player->isSprinting()))
					Util::sendPacket(PlayerActionPacket(Util::player->getRuntimeID(), PlayerActionPacket::Action::StopSprinting));

				// Swing
				if (*swing && (otherSwing = !otherSwing))
					Util::sendPacket(AnimatePacket{player.getRuntimeID(), AnimatePacket::Action::SwingArm});

				for (int i = 0; i < *attacks; ++i) {
					// Attack
					if (*legacy)
						Util::sendPacket(InteractPacket{InteractPacket::Action::LegacyAttack, player.getRuntimeID(), 0, 0, 0});
					else
						Util::sendPacket(AnonymousPacket{"InventoryTransactionPacket", 0x1e, [&](BinaryStream &stream) {
							stream.writeVarInt(0); // no changed item bs
							stream.writeUnsignedVarInt(3); // use item on entity
							stream.writeBool(false);
							stream.writeUnsignedVarInt(0); // action count
							// no actions
							stream.writeUnsignedVarInt64(player.getRuntimeID()); // entity id
							stream.writeUnsignedVarInt(1); // attack entity (0 is interact)
							stream.writeVarInt(Util::player->getSelectedItemSlot()); // hotbar slot (0-8)
							serialize<ItemStack>::write(*Util::player->getInventoryMenu().getSlot(Util::player->getSelectedItemSlot()), stream); // write item
							// below are little endian floats
							// player position (you)
							stream.writeVec3(sPos);
							// click position (all 0s for this type)
							stream.writeFloat(0);
							stream.writeFloat(0);
							stream.writeFloat(0);
						}}, true);
						Util::player->attack(player);
				}

				if (*slow && Util::player->isSprinting()) {
					Util::player->motion = Util::player->motion.multiply(0.8, 1);
					Util::player->setSprinting(false);
				} else if (resprint) {
					Util::sendPacket(PlayerActionPacket{Util::player->getRuntimeID(), PlayerActionPacket::Action::StartSprinting});
				}

				if (!*multi)
					return false; // Break

				return true; // Continue
			}, [&](Player &player) {
				bool valid = Util::valid(player, true, *range, *fov) && targetCount < *targets && (!*tick || !player.hurtTime) && (!*blockRaytrace || Util::player->canSee(player)) /*&& (!*antiBot || (player.ticksExisted > 400 && std::find(playersSwung.begin(), playersSwung.end(), player.getRuntimeID().data) != playersSwung.end()))*/;
				if (valid)
					++targetCount;
				return valid;
			}, FOVComparator{})) { // Sort by closest to cursor
		 	// If player list copy was empty:
				if (*swing)
					Util::forValidPlayers([&](Player &player) {
						if (Util::valid(player, true, *swingRange, *swingFov)) {
							Util::player->swing();
							nextDelay = 1000 / *swingSpeed;
							lastTime = curTime;
							return false;
						}
						return true;
					});
			} else {
				nextDelay = 1000 / *speed;
				lastTime = curTime;

				if (teleported) {
					if (*teleportReturn)
						Util::stepTeleport(sPos, Util::player->getPos(), {*teleportSpeed, 1}, 3);
					else
						Util::player->setPos(sPos);
				}

				if (oldHeld != -1) {
					Util::player->setSelectedItemSlot(oldHeld);
					Util::sendPacket(EquipPacket{oldHeld});
				}

				if (*silentAim)
					Util::sendPacket(MovePlayerPacket{Util::player->getRuntimeID(), Util::player->onGround, Util::player->getPos(), Vec3{Util::player->getRotation()}}, true);
			}
		}
	});

	addDynamicListener<PreReadEvent<Packet>>([](auto &pe) {
		// idfk equipment packets fuck up something ig
		if (pe.packet.getId() == 0x1f)
			pe.cancel();
	});
}
