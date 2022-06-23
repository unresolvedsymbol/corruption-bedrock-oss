#include <network/protocol/MovePlayerPacket.h>
#include <network/protocol/AnimatePacket.h>
#include <network/protocol/AddItemActorPacket.h>
#include "AutoHeal.h"
#include "packet/EquipPacket.h"
#include "packet/AnonymousPacket.h"
#include "packet/StartUsePacket.h"
#include "events/impl/PacketEvents.h"
#include "events/impl/PlayerEvents.h"
#include "packet/ReleasePacket.h"


bool AutoHeal::validEffects(short data) {
	return data == 21 || data == 22 || // Instant health
	// Regeneration (while not currently having it)
	(data > 27 && data < 31 && !Util::player->hasEffect(*MobEffect::REGENERATION));
}

AutoHeal::AutoHeal() : ToggleModule("AutoHeal") {
	addDynamicListener<MoveEvent>([&](auto &me) {
		if (delay < 1) {
			switch (stage) {
				case 0: {
					if (Util::player->getHealth() > *health || Util::player->isUsingItem())
						break;

					oldStack = Util::player->getInventoryMenu().getSlot(oldSlot = Util::player->getSelectedItemSlot());

					for (slot = 0; slot < 9; ++slot) {
						stack = Util::player->getInventoryMenu().getSlot(slot);

						if (stack->isNull())
							continue;

						// Prefer the least work
						if (*soup && stack->getId() == 282)
							mode = 2;
						else if (*splash && (stack->getId() == 466 || stack->getId() == 438) && validEffects(stack->getAuxValue()))
							mode = 0;
						else if ((*bottle && stack->getId() == 373 && validEffects(stack->getAuxValue())) || (*gapple && stack->getId() == 322))
							mode = 1;
						else
							continue;

						// Switch to slot with desired healing
						Util::sendPacket(EquipPacket{slot, stack});

						// Hold for a few before action
						delay = *hold;
						stage = 1;

						break;
					}
				} break;
				case 1: {
					switch (mode) {
						case 0: { // Click use with aim (Splash)
							bool canUp = *up || *clip;

							// Make sure the area above is clear so it doesn't hit a block wasting it
							if (canUp)
								for (int i = 0; i < 4; ++i)
									if (!Util::player->getRegion().getMaterial(Util::player->getPos().add(0, 1, 0)).isReplaceable())
										canUp = false;

							// Aim
							MovePlayerPacket move{Util::player->getRuntimeID(), Util::player->onGround, Util::player->getPos(), Vec3{Util::player->getRotation()}};
							move.pitch = canUp ? -90 : 90;
							Util::sendPacket(move);

							// Swing
							Util::sendPacket(AnimatePacket{Util::player->getRuntimeID(), AnimatePacket::Action::SwingArm});

							// Use
							Util::sendPacket(StartUsePacket{slot});

							// Jump
							if (canUp && *jump && Util::player->onGround)
								Util::player->motion.y = me.motion.y = .425;

							if (canUp)
								slow = 5;
						} break;

						case 1: { // Fast Consume/Eat/Drink (Gapples/Bottled Potions)
							// TODO: Normal slow consume
							Util::sendPacket(StartUsePacket{slot});
							Util::sendPacket(ReleasePacket{});
						} break;

						case 2: { // Click use without aim (Soup etc)
							Util::sendPacket(StartUsePacket{slot});
						} break;
					}

					// Switch back to previous slot
					Util::sendPacket(EquipPacket{oldSlot, oldStack});

					// Extra step for the untested potion catcher
					stage = (!mode && *clip) ? 2 : 0;

					delay = *wait;

					// Wait at least a couple seconds with potions and gapples especially
					if (mode == 1)
						delay = delay < 40 ? 40 : delay;

					break;
				}
			}
		} else
			--delay;

		if (slow > 0) {
			me.motion.x = std::min(me.motion.x, .1f);
			me.motion.z = std::min(me.motion.z, .1f);
			slow--;
		}
	}, EventPriority::Lowest)->addDynamicListener<PreReadEvent<AddItemActorPacket>>([&](auto &pe) {
		if (stage == 2 && pe.packet.stack.getId() == 438 && validEffects(pe.packet.stack.getAuxValue()) && pe.packet.pos.distance(Util::player->pos) < 3.5) {
			// Grab that shit
			Util::player->setPos(pe.packet.pos.add(pe.packet.motion));
			stage = 0;
		}
	});
}
