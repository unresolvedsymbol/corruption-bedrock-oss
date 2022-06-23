#include "world/VanillaBlocks.h"
#include "network/protocol/PlayerActionPacket.h"
#include "network/protocol/ActorEventPacket.h"
#include "network/protocol/MovePlayerPacket.h"
#include "packet/AnonymousPacket.h"
#include "entity/effect/MobEffect.h"
#include "item/Enchant.h"
#include "item/EnchantUtils.h"
#include "network/protocol/SetActorDataPacket.h"
#include "network/protocol/TextPacket.h"
#include "network/protocol/MobEffectPacket.h"
#include "network/protocol/AnimatePacket.h"
#include "network/protocol/LevelEventPacket.h"
#include "packet/StartUsePacket.h"
#include "packet/ReleasePacket.h"
#include "packet/EquipPacket.h"
#include "events/impl/MiscEvents.h"
#include "events/impl/PacketEvents.h"
#include "events/impl/PlayerEvents.h"
#include "util/StaticHook.h"
#include "SimpleModules.h"
#include "Corruption.h"
#include "client/input/Keyboard.h"
#include <iomanip>
#include <network/protocol/MoveActorAbsolutePacket.h>
#include <network/protocol/LoginPacket.h>

extern char base;


struct ChestBlockActor {
	void setItem(int slot, const ItemStack &);

	void saveItems(CompoundTag &);
};

struct SignBlockActor {
	void setMessage(std::string, std::string);
};

NoSlow::NoSlow() : ToggleModule("NoSlow", true) {
	addDynamicListener<SlowdownEvent>([](auto &se) {
		se.cancel();
	})->addDynamicListener<MoveInputEvent>([](auto &te) {
		if (!Util::player->isUsingItem())
			return;
		if (Util::input->forward && !Util::input->strafe) {
			Util::input->forward /= .35;
		} else if (Util::input->strafe && !Util::input->forward) {
			Util::input->strafe /= .35;
		} else if (Util::input->strafe && Util::input->forward) {
			Util::input->strafe /= .27;
			Util::input->forward /= .27;
		}
	});
}

PacketLogger::PacketLogger() : ToggleModule("PacketLogger", false) {
	addDynamicListener<PreSendEvent<Packet>>([](auto &event) {
		Log(1) << "sent " << event.packet.getName() << " [" << std::hex << event.packet.getId() << "]"
			   << (event.cancelled ? " (cancelled)" : "");
	}, EventPriority::Lowest)->addDynamicListener<PreReadEvent<Packet>>([](auto &event) {
		Log(1) << "read " << event.packet.getName() << " [" << std::hex << event.packet.getId() << "]"
			   << (event.cancelled ? " (cancelled)" : "");
	}, EventPriority::Lowest);
}

// Spawn items on Nukkit, exploit removed
ItemSmith::ItemSmith() : ToggleModule("ItemSmith") { 
	addDynamicListener<ItemHook>([&](auto &ih) {
		if (id)
			ih.stack._setItem(id);
		if (count)
			ih.stack.set(count);
		if (data || ih.stack.isDamageableItem())
			ih.stack.setDamageValue(data);
		for (int i = 0; i < 33; ++i)
			if (enchants[i])
				EnchantUtils::applyEnchant(ih.stack, static_cast<Enchant::Type>(i), enchants[i], false);
	})->addModule((new Module("Enchant"))->setRun([&](auto, args_t args) {
		int i = 0;
		short id, lvl;
		while (Util::parse(args, i++, id))
			if (Util::parse(args, i++, lvl)) {
				enchants[id] = lvl;
				Log() << "Set " << id << " to " << lvl;
			}
		Log() << "Set enchants";
	}))->addModule((new Module("Set"))->setRun([&](auto, args_t args) {
		for (int i = 0; i < 33; ++i) enchants[i] = 0;
		id = count = data = 0;
		int xcount = 0, xdata = 0;
		Util::parse(args, 0, id);
		Util::parse(args, 1, xcount);
		Util::parse(args, 2, xdata);
		// fuck this
		count = xcount;
		data = xdata;
		Log() << "Set item [" << id << ", " << +data << ", " << +count << "]";
	}))->addModule((new Module("Clear"))->setRun([&](auto, args_t args) {
		for (int i = 0; i < 33; ++i) enchants[i] = 0;
		id = count = data = 0;
		Log() << "Cleared";
	}));
}

JetPack::JetPack() : ToggleModule("JetPack") {
	addDynamicListener<MoveEvent>([](auto &me) {
		if (Util::input->forward)
			me.motion = Util::getLookVec(Util::player->rotation);
	});
}

// TODO: Use real block AABB events instead of this barrier block bullshit
WaterWalk::WaterWalk() : ToggleModule("WaterWalk") {
	addDynamicListener<MoveEvent>([](auto &me) {
		//auto move = Util::getMoveVec(.6f);
		BlockPos pos = Util::player->aabb.min.add(.5, -.001, .5).add(me.motion);
		for (int i = 0; i < 5; ++i) {
			if (Util::player->getRegion().getMaterial(pos).isType((MaterialType) 5)) {
				Util::player->getRegion().setBlock(pos, *VanillaBlocks::mBarrierBlock);
				break;
			}
			pos.y--;
		}
	}, EventPriority::Low)->addDynamicListener<PreSendEvent<MovePlayerPacket>>([](auto &pe) {
		if (Util::player->onGround && Util::player->getRegion().getBlock(Util::player->aabb.min.subtract(0, .001, 0)) == VanillaBlocks::mBarrierBlock) {
			if (Util::player->fallDistance > 3) {
				pe.packet.y -= .3;
				Util::player->fallDistance = 0;
			} else {
				pe.packet.y -= Util::player->isOnFire ? .3 : .2;
			}
		}
	});
}

EggAura::EggAura() : ToggleModule("EggAura") {
	addDynamicListener<PlayerTick>([](auto &) {
		static int delay = 0;
		if (delay < 1)
			Util::player->getLevel().forEachPlayer([](Player &player) {
				//auto size = player.aabb.max.subtract(player.aabb.min);
				if (player.getRuntimeID() == Util::player->getRuntimeID()
					|| Util::player->distanceTo(player) > 8
					|| player.getUnformattedNameTag().find(" Egg") == std::string::npos)
					return true;

				auto blockPos = BlockPos{player.getPos().subtract(0, 2, 0)};

				if (Util::player->getRegion().getBlock(blockPos) == VanillaBlocks::mDragonEgg) {
					// Make sure Egg isn't covered
					for (unsigned char face = 0; face < 4; ++face) {
						if (Util::player->getRegion().getMaterial(blockPos.neighbor(face)).isReplaceable()) {
							PlayerActionPacket eggPacket{Util::player->getRuntimeID(),
														 PlayerActionPacket::Action::StartBreak};
							face += face % 2 == 0 ? 1 : -1;
							eggPacket.face = face;
							eggPacket.blockPos = blockPos;
							Util::sendPacket(eggPacket);
							eggPacket.action = PlayerActionPacket::Action::StopBreak;
							Util::sendPacket(eggPacket);

							delay = 6;

							return false;
						}
					}

					for (unsigned char face = 0; face < 4; ++face) {
						if (!Util::player->getRegion().getMaterial(blockPos.neighbor(face)).isReplaceable()) {
							Util::player->gameMode->destroyBlock(blockPos.neighbor(face), face + (face % 2 == 0 ? 1 : -1));

							std::string faceName{Direction::names[face]};
							std::transform(faceName.begin(), faceName.end(), faceName.begin(), ::tolower); // god save us :^)
							// correct grammar: north of egg vs. below/above egg
							Log() << "Breaking block " << faceName << (face < 2 ? "" : " of") << " egg";

							delay = 160;
						}
					}
				}

				return true;
			});
		else
			--delay;
	});
}

Teams::Teams() : ToggleModule("Teams") {}

AutoTotem::AutoTotem() : ToggleModule("AutoTotem") {
	static const auto spawnTotemVanilla = []() {
		// Exploit removed
	};
	addDynamicListener<PreReadEvent<ActorEventPacket>>([](auto &pe) {
		if (pe.packet.eid == Util::player->getRuntimeID() && (pe.packet.event == 65 || pe.packet.event == 2 || pe.packet.event == 3)) // Use totem or just hurt in general, pretty fuckin agro
			spawnTotemVanilla();
	});
}


NoHunger::NoHunger() : ToggleModule("NoHunger") {
	addDynamicListener<PreSendEvent<PlayerActionPacket>>([](auto &pe) {
		if (pe.packet.action == PlayerActionPacket::Action::Jump || pe.packet.action == PlayerActionPacket::Action::StartSprinting)
			pe.cancel();
	})->addDynamicListener<PreSendEvent<MovePlayerPacket>>([](auto &pe) {
		pe.packet.onGround = false;
		/*if (Util::player->ticksExisted % 3 != 0)
			pe.cancel();*/
	});
}

Swim::Swim() : ToggleModule("SpoofSwim") {
	addDynamicListener<InWaterEvent>([](auto &e) {
		e.value = true;
	});
}

SpoofElytra::SpoofElytra() : ToggleModule("SpoofElytra") {
	addDynamicListener<PreSendEvent<MovePlayerPacket>>([](auto &pe) {
		if (!Util::player->isGliding()) {
			Util::sendPacket(PlayerActionPacket{Util::player->getRuntimeID(), PlayerActionPacket::Action::StartGlide});
			//Util::sendPacket(pe.packet);
			Util::player->startGliding();
		}
	});
	setOnDisable([]() {
		Util::sendPacket(PlayerActionPacket{Util::player->getRuntimeID(), PlayerActionPacket::Action::StopGlide});
	});
}

// This doesn't actually find anything interesting. Sent by players in the same chunk.
Sniffer::Sniffer() : ToggleModule("Sniffer") {
	addDynamicListener<PreReadEvent<LevelEventPacket>>([](auto &pe) {
		/*switch (pe.packet.sound) {
			case 5:
				Log() << "Block was broke at " << pe.packet.pos.x << " " << pe.packet.pos.y << " " << pe.packet.pos.z;
			case 6:
				Log() << "Block was placed at " << pe.packet.pos.x << " " << pe.packet.pos.y << " " << pe.packet.pos.z;
				break;
			case 55:
				Log() << "Block was broke at " << pe.packet.pos.x << " " << pe.packet.pos.y << " " << pe.packet.pos.z;
			default:
				Log(1) << "Unknown sound " << pe.packet.sound << " at " << pe.packet.pos.x << " " << pe.packet.pos.y << " " << pe.packet.pos.z;
				break;
		}*/
		Log() << pe.packet.event << " " << pe.packet.pos.x << " " << pe.packet.pos.y << " " << pe.packet.pos.z;
	});
	addDynamicListener<PreReadEvent<Packet>>([](auto &pe) {
		//if (pe.packet.getId() == 0x38) // BlockActorData
		// TODO Sign update sniffer
		//Log() << "Actor event " << +pe.packet.event << " for ID " << pe.packet.eid.data;
	});
}

AutoFish::AutoFish() : ToggleModule("AutoFish") {
	addDynamicListener<PostReadEvent<ActorEventPacket>>([&](auto &pe) {
		if (stage == 0 && pe.packet.event == 13) { // Fish hook hooked (bite)
			stage = 1;
			delay = 2;
		}
	});
	addDynamicListener<PlayerTick>([&](auto &pe) {
		if (delay > 0)
			--delay;
		else {
			switch (stage) {
				case 1:
					Util::sendPacket(StartUsePacket{}); // Reel
					stage = 2;
					delay = 3;
					break;
				case 2:
					Util::sendPacket(StartUsePacket{}); // Throw again
					stage = 0;
					delay = 5;
					break;
			}
		}
	});
}


FastEat::FastEat() : ToggleModule("FastEat") {
	// Exploit Removed
}

ServerCrasher::ServerCrasher() : ToggleModule("Crasher") {
	addDynamicListener<PlayerTick>([](auto &pe) {
		// Exploit removed
	});
}

AutoLog::AutoLog() : ToggleModule("AutoLog") {
	addDynamicListener<PlayerTick>([](auto &) {
		static unsigned short delay = 0;
		if (!delay) {
			if (Util::client->isPlaying() && !Util::client->isExitingLevel() && Util::player->getHealth() < 6) {
				Util::game->startLeaveGame();
				Log() << "Disconnected at " << Util::player->getPos();
				delay = 600; // 30 seconds
			}
		} else if (Util::game->isLeaveGameDone())
			--delay;
	});
}

Criticals::Criticals() : ToggleModule("Criticals") {
	// Exploit removed
}

// Literally packet loss.
// Not lag, you literally teleport. And for some reason this breaks movement checks on hive and broken lens.
PacketLoss::PacketLoss() : ToggleModule("PacketLoss") {
	// Exploit removed
}

FastClimb::FastClimb() : ToggleModule("FastClimb") {
	addDynamicListener<MoveEvent>([](auto &me) {
		// TODO: Don't spider, check for ladders in interpolated BB
		if (Util::player->collidedHorizontally && (Util::input->forward || Util::input->strafe)) {
			Util::player->aabb.offset(0, .35f, 0);
			me.motion.x = me.motion.z = 0; // Don't speed off the ladder
		}
	}, EventPriority::Low);
}

Blink::Blink() : ToggleModule("Blink") {
	addDynamicListener<PreSendEvent<MovePlayerPacket>>(
			[&](PreSendEvent<MovePlayerPacket> &pe) {
				moves.emplace_back(pe.packet.onGround, pe.packet.pos, pe.packet.rot);
				pe.cancel();
			})->setOnDisable([&]() {
				Log() << "Sent " << moves.size() << " moves";
				for (auto it = moves.begin(); it != moves.end(); it = moves.erase(it)) {
					Util::sendPacket(MovePlayerPacket{Util::player->getRuntimeID(), std::get<0>(*it), std::get<1>(*it),
													  std::get<2>(*it)}, false);
					Log(1) << "send";
				}
	});
}

Freecam::Freecam() : ToggleModule("Freecam") {
	addDynamicListener<PreSendEvent<MovePlayerPacket>>([](auto &pe) {
		pe.cancel();
	})->addDynamicListener<PreSendEvent<AnimatePacket>>([](auto &pe) {
		pe.cancel();
	})->addDynamicListener<PreSendEvent<PlayerActionPacket>>([](auto &pe) {
		pe.cancel();
	})->setOnEnable([&]() {
		if (Util::client->isPlaying()) {
			oldPosition = Util::player->getPos().add(0, 0.0001, 0);
			Util::player->setPlayerGameTypeWithoutServerNotification(GameType::SURVIVAL_SPECTATOR);
		}
	})->setOnDisable([&]() {
		if (Util::client->isPlaying()) {
			Util::player->setPlayerGameTypeWithoutServerNotification(Util::player->getLevel().getDefaultGameType());
			Util::player->setPos(oldPosition);
		}
	})->addDynamicListener<MoveEvent>([](auto &me) {
		auto move = Util::getMoveVec(.8f);
		me.motion = {move.x, Util::input->sneak ? -1 : Util::input->jump ? 1 : 0.f, move.y};
	});
}

// TODO instant break, hook start destroy

ElytraFly::ElytraFly() : ToggleModule("ElytraFly") {
	addDynamicListener<PlayerTick>([](auto &) {
		if (Util::player->isGliding() && Util::player->getGlidingTicks() > 12) {
			if (std::abs(Util::player->motion.x + Util::player->motion.z) > 2 && Util::player->rotation.pitch < 0) {
				Util::player->motion.x /= 1.7f;
				Util::player->motion.z /= 1.7f;
				Util::player->motion.y *= 1.0001f;
			} else if (Util::player->rotation.pitch > 0) {
				Util::player->motion.y = 0;
				auto move = Util::getMoveVec(2.5f);
				Util::player->motion.x = move.x;
				Util::player->motion.z = move.y;
			}
			if (Util::input->sneak)
				Util::player->motion.y = -.6f;
			else if (Util::input->jump)
				Util::player->motion.y = .6f;
		}
	});
}

// Vanilla meme, doesn't work on PMMP or Nukkit because you can't move while dead or sleeping. Need to look for an exploit on those.
GodMode::GodMode() : ToggleModule("GodMode") {
	addDynamicListener<PreSendEvent<PlayerActionPacket>>([](auto &pe) {
		if (pe.packet.action == PlayerActionPacket::Action::Respawn || pe.packet.action == PlayerActionPacket::Action::StopSleeping)
			pe.cancel();
	});
	setOnDisable([]() {
		Util::sendPacket(PlayerActionPacket{Util::player->getRuntimeID(), PlayerActionPacket::Action::Respawn});
		Util::sendPacket(PlayerActionPacket{Util::player->getRuntimeID(), PlayerActionPacket::Action::StopSleeping});
	});
}

Sneak::Sneak() : ToggleModule("Sneak") {
	addDynamicListener<PreSendEvent<PlayerActionPacket>>([](auto &event) {
		// Server gets pissy if you don't send the exact opposite (to fix client desync or something)
		if (event.packet.action == PlayerActionPacket::Action::StartSneaking ||
			event.packet.action == PlayerActionPacket::Action::StopSneaking)
			event.cancel();
	// TODO: Fix drugs
	})->addDynamicListener<PreReadEvent<SetActorDataPacket>>([](auto &pe) {
		if (Util::player->getRuntimeID() != pe.packet.rid)
			return;
		for (const auto &entry : pe.packet.entries)
			if (entry->id == 1) { // Sneaking flag
				if (!entry->byteVal)
					Util::sendPacket(PlayerActionPacket{Util::player->getRuntimeID(), PlayerActionPacket::Action::StartSneaking});
				entry->byteVal = 0;
			}
	// Respawn packets
	})->addDynamicListener<PreReadEvent<Packet>>([](auto &pe) {
		if (pe.packet.getId() == 0x2d)
			Util::sendPacket(PlayerActionPacket{Util::player->getRuntimeID(), PlayerActionPacket::Action::StartSneaking});
	})->addDynamicListener<PreSendEvent<Packet>>([](auto &pe) {
		if (pe.packet.getId() == 0x2d)
			Util::sendPacket(PlayerActionPacket{Util::player->getRuntimeID(), PlayerActionPacket::Action::StartSneaking});
	})->setOnEnable([]() {
		Util::sendPacket(PlayerActionPacket{Util::player->getRuntimeID(), PlayerActionPacket::Action::StartSneaking});
	});
	// TODO: Unsneak for opening containers
}

// Also hides sprint particles, tested on nukkit and pmmp
NoShadow::NoShadow() : ToggleModule("NoShadow") {
	// Exploit removed
}

FallBack::FallBack() : ToggleModule("FallBack") {
	addDynamicListener<MoveEvent>([](auto &me) {
		static Vec3 groundPos = Vec3::ZERO;
		if (Util::player->onGround && !Util::player->getRegion().getMaterial(Util::player->getPos().subtract(0, 2, 0)).isReplaceable())
			groundPos = Util::player->getPos();
		else if (groundPos != Vec3::ZERO && Util::player->fallDistance > 3) {
			for (int dist = 1; dist < 10; dist++)
				if (!Util::player->getRegion().getMaterial(Util::player->getPos().subtract(0, 2 + dist, 0)).isReplaceable())
					return;
			Util::player->setPos(groundPos);
			Util::player->motion = me.motion = Vec3::ZERO;
		}
	});
}

// Reverse step but also when coming down from flight etc
FastFall::FastFall() : ToggleModule("FastFall") {
	addDynamicListener<MoveEvent>([](auto &me) {
		if (!Util::player->onGround && !Util::player->hasEffect(*MobEffect::JUMP) && Util::player->fallDistance > 0 && me.motion.y < 0)
			for (BlockPos pos = Util::player->aabb.min; pos.y > 0; --pos.y)
				if (!Util::player->getRegion().getMaterial(pos).isReplaceable()) {
					if ((Util::player->pos.y - pos.y) > 8) // Check for block not in void and at least 3 blocks from feet (+ 2 cause head pos)
						me.motion.y = -99;
					return;
				}
	}, EventPriority::Low);
}

// Only for Steadfast afaik, PMMP and Nukkit seem to sucessfully allow clipping hundreds of blocks on the Y coordinate therefore counting up fall distance eventually.
SafeWalk::SafeWalk() : ToggleModule("SafeWalk") {
	addDynamicListener<MoveEvent>([](auto &me) {
		if (Util::player->onGround && Util::player->collidedVertically) {
			auto move = Util::getMoveVec(.6);
			for (int i = 0; i < 10; ++i)
				if (!Util::player->getRegion().getMaterial(
						Util::player->aabb.min.add(move.x, me.motion.y - i, move.y)).isReplaceable())
					return;
			auto revMove = Util::getMoveVec(-.6);
			me.motion.x = revMove.x;
			me.motion.z = revMove.y;
		}
	}, EventPriority::Low);
}

AutoSell::AutoSell() : ToggleModule("AutoSell") {
	addDynamicListener<PlayerTick>([](auto &) {
		const static std::unordered_set<short> canSell{
			// Basics
			3,
			4,
			17,
			// Ingots
			263,
			264,
			265,
			266,
			351, // Dye, hopefully lapiz
			// Blocks
			173,
			152,
			133,
			57,
			42,
			41,
			22,
		};
		static unsigned char delay = 0, stage = 0;
		static short slot = 0;
		static std::vector<short> toSell;

		if (delay > 0)
			delay--;
		else {
			switch (stage) {
				case 0: {
					for (int i = 9; i < 36; ++i)
						if (auto stack = Util::player->getInventoryMenu().getSlot(i))
							if (stack && stack->isNull()) {
								delay = 5;
								return;
								// Inventory has space, sleep
							}

					Log() << "Starting AutoSell...";

					toSell.clear();

					for (short i = 9; i < 36; ++i)
						if (auto stack = Util::player->getInventoryMenu().getSlot(i))
							if (canSell.find(stack->getId()) != canSell.end())
								toSell.push_back(i);

					stage = 1;
					slot = 0;
				} break;
				case 1: {
					if (slot < 9) {
						Util::sendPacket(EquipPacket{slot});
						Util::sendPacket(TextPacket{TextPacketType::CHAT, "", "/sh", {}, false, "", ""});
						// TODO: Send "/sh" chat packet
						slot++;
						delay = 3;
					} else {
						if (toSell.empty()) {
							Log() << "Finished selling.";
							stage = slot = 0;
							delay = 5;
							return;
						}

						auto moving = toSell.begin();
						if (auto movingStack = Util::player->getInventoryMenu().getSlot(*moving)) {
							if (movingStack && !movingStack->isNull()) {
								for (short i = 0; i < 9; ++i)
									if (auto hotbar = Util::player->getInventoryMenu().getSlot(i))
										if (!hotbar || hotbar->isNull()) {
											// Quick move from inventory to hotbar
											Util::sendPacket(AnonymousPacket{"InventoryTransactionPacket", 0x1e, [&](BinaryStream &stream) {
												stream.writeUnsignedVarInt(0); // transaction type
												stream.writeUnsignedVarInt(2); // transaction count
												// begin action 1
												stream.writeUnsignedVarInt(0); // source type
												stream.writeVarInt(0); // window id (inv)
												stream.writeUnsignedVarInt(*moving); // inventory slot
												// begin custom itemstack (oldItem)
												serialize<ItemStack>::write(*movingStack, stream);
												// begin custom itemstack (newItem)
												stream.writeVarInt(0); // id, don't write anything else if empty
												// end action 1
												// begin action 2
												stream.writeUnsignedVarInt(0); // source type
												stream.writeVarInt(0); // window id (inv)
												stream.writeUnsignedVarInt(i); // inventory slot
												// begin custom itemstack (oldItem)
												stream.writeVarInt(0); // id
												// begin custom itemstack (newItem)
												serialize<ItemStack>::write(*movingStack, stream);
												// end action 2
											}});

											Util::player->getSupplies().swapSlots(*moving, i);

											moving = toSell.erase(moving);

											break;
										}
							}
						}
					}
				} break;
			}

			delay = 3;
		}
	});
}

DeathCoords::DeathCoords() : ToggleModule("DeathLog") {
	// Normal death
	addDynamicListener<PreReadEvent<SetActorDataPacket>>([](auto &pe) {
		if (Util::client->isPlaying() && pe.packet.rid == Util::player->getRuntimeID())
			for (DataItem *item : pe.packet.entries)
				if (item->id == 1)
					if (item->intVal == 0) // Health
						Log() << "Died at " << Util::player->prevPos.x << " " << Util::player->prevPos.y << " "
							  << Util::player->prevPos.z;
	});

	// Instant death (auto respawn plugins)
	addDynamicListener<PreReadEvent<Packet>>([](auto &pe) {
		if (pe.packet.getId() == 0x2b && Util::client->isInGame()) // Set spawn position packet because the respawn packet is late way more often
			Log() << "Died at " << round(Util::player->prevPos.x) << " " << round(Util::player->prevPos.y) << " " << round(Util::player->prevPos.z);
	});
}

AntiEffect::AntiEffect() : ToggleModule("AntiEffect", true) {
	addDynamicListener<PreReadEvent<MobEffectPacket>>([](auto &pe) {
		if (MobEffect::getById(pe.packet.effectID)->isHarmful())
		/*if (pe.packet.effectID == 9 || pe.packet.effectID == 15 ||
			pe.packet.effectID == 4 || pe.packet.effectID == 18 ||
			pe.packet.effectID == 20 || pe.packet.effectID == 27 ||
			pe.packet.effectID == 2 || pe.packet.effectID == 31)*/
			pe.cancel(); // Fuck your effects pay to win niggers
	});
}

Name::Name() : Module("Name") {
	setRun([](auto, args_t args) {
		std::string name;
		if (Util::parseRest(args, 0, name))
			Util::player->getInventoryMenu().getSlot(Util::player->getSelectedItemSlot())->setCustomName(name);
		else
			Log() << "Usage: .name <name for held item>";
	});
}

Particles::Particles() : ToggleModule("Particles") {
	addDynamicListener<PlayerTick>([&](auto &) {
		static unsigned char tick = 0;
		if ((Util::input->forward || Util::input->strafe)) {
			if (!tick) {
				auto pk = ActorEventPacket(Util::player->getRuntimeID(), 57, *item << 16);
				for (int i = 0; i < *multiplier; ++i)
					Util::sendPacket(pk);
				tick = *delay;
			} else
				tick--;
		}
	});
}

Zoom::Zoom() : ToggleModule("Zoom") {
	addDynamicListener<FOVEvent>([](auto &event) {
		event.modifier = -3.f;
	});
}

InventoryMove::InventoryMove() : ToggleModule("InvMove", true) {
	addDynamicListener<MoveInputEvent>([](auto &) {
		if (!Util::client->isPlaying() || Util::client->isInGame())
			return;
		//Util::input->forward = Util::input->strafe = 0;
		//if (Util::input->up)
		if (Keyboard::_states['W'])
			++Util::input->forward;
		//if (Util::input->down)
		if (Keyboard::_states['S'])
			--Util::input->forward;
		if (Keyboard::_states['A'])
		//if (Util::input->left)
			--Util::input->strafe;
		//if (Util::input->right)
		if (Keyboard::_states['D'])
			++Util::input->strafe;
	});
}

AntiAim::AntiAim() : ToggleModule("AntiAim", true) {
	addDynamicListener<PreReadEvent<MoveActorAbsolutePacket>>([](auto &pe) {
		if (Util::player && pe.packet.rid == Util::player->getRuntimeID()) {
			pe.packet.pitch = Util::player->rotation.pitch / (360 / 256);
			pe.packet.yaw = pe.packet.headYaw = Util::player->rotation.yaw / (360 / 256);
		}
	});
	addDynamicListener<PreReadEvent<MovePlayerPacket>>([](auto &pe) {
		if (Util::player && pe.packet.runtimeID == Util::player->getRuntimeID()) {
			pe.packet.rot = Vec3{Util::player->rotation};
			if (pe.packet.mode == MovePlayerPacket::Mode::Reset)
				pe.packet.mode = MovePlayerPacket::Mode::Normal;
		}
	});
}

HighJump::HighJump() : ToggleModule("HighJump") {
	static bool jumping;
	addDynamicListener<MoveEvent>([](auto &me) {
		if (!Util::player->onGround && (Util::input->forward != 0 || Util::input->strafe != 0) && !Util::player->collidedHorizontally)
			Util::player->motion.y += 0.05999;
		else
			jumping = false;

		//me.setSpeed(Util::getBaseMoveSpeed() * 1.08f);
		if (jumping) {
			auto move = Util::getMoveVec(Util::getBaseMoveSpeed() * 1.08f);
			me.motion.x = move.x;
			me.motion.z = move.y;
		}
	});
	addDynamicListener<MoveInputEvent>([](auto &mie) {
		if (!jumping && Util::input->jump) {
			jumping = true;
			Util::input->jump = false;
		}
	});
}

Derp::Derp() : ToggleModule("Derp") {
	addDynamicListener<PreSendEvent<MovePlayerPacket>>([&](auto &pe) {
		pe.packet.yaw = pe.packet.headYaw = static_cast<int>(pe.packet.pitch) % 360 + 15;
		pe.packet.pitch = -180;
	});
}