#include "Scaffold.h"
#include "events/impl/MiscEvents.h"
#include "packet/EquipPacket.h"

Scaffold::Scaffold() : ToggleModule("Scaffold") {
	addDynamicListener<MoveEvent>([&](auto &me) {

		// Too simple hat scaffold that was fixed
		/*BlockPos block = Vec3{Util::player->pos.x + me.motion.x,
							  Util::player->aabb.min.y + me.motion.y - 0.001f,
							  Util::player->pos.z + me.motion.z};

		if (Util::player->getSelectedItem().isBlock() &&
			Util::player->getRegion().getBlock(block).getMaterial().isReplaceable()) {
			Util::player->swing();
			unsigned char face = Facing::LookDirection::toDirection(Util::player->getDirection()),
					flip = face + (face % 2 == 0 ? 1 : -1);
			Util::player->gameMode->buildBlock(block.neighbor(flip), face);
		}
		return;*/

		short pseudoSlot = -1, realSlot = Util::player->getSelectedItemSlot();
		ContainerItemStack *stack;
		if (!Util::player->getSelectedItem().isBlock()) {
			if (!*silent)
				return;
			for (int slot = 0; slot < 9; ++slot) {
				stack = Util::player->getInventoryMenu().getSlot(slot);
				if (!stack ||
					stack->isNull() ||
					!stack->isBlock())
					continue;
				pseudoSlot = slot;
				break;
			}
			if (pseudoSlot == -1)
				return;
		}

		BlockPos x = Vec3{Util::player->pos.x + me.motion.x, Util::player->aabb.min.y + me.motion.y - .001f, Util::player->pos.z + me.motion.z};
		unsigned char face = Direction::INVALID;

		if (Util::player->getRegion().getMaterial(x).isReplaceable()) {
			for (unsigned char y = Direction::DOWN; y <= Direction::EAST; ++y) {
				if (!Util::player->getRegion().getMaterial(x.neighbor(y)).isReplaceable()) {
					face = y;
					break;
				}
			}
		}

		if (face == Direction::INVALID) {
			unsigned char look = Direction::convertFacingDirectionToDirection(
					static_cast<unsigned char>(Util::player->getDirection()));
			look += look % 2 == 0 ? 1 : -1;
			if (Util::input->forward > 0 && Util::player->getRegion().getMaterial(x.neighbor(look)).isReplaceable())
				face = look;
			else
				return;
		} else {
			x = x.neighbor(face);
			face += face % 2 == 0 ? 1 : -1;
		}

		if (pseudoSlot != -1) {
			Util::player->setSelectedItemSlot(pseudoSlot);
			Util::sendPacket(EquipPacket{pseudoSlot, stack});
		}

		Util::player->gameMode->buildBlock(x, face);

		//if (Util::input->jump && (face == Facing::Direction::UP && *tower) || (Util::input->forward && Util::player->gameMode->buildBlock(x.relative(Facing::Direction::UP, 1), Facing::Direction::DOWN))) { // Tower
		if (!Util::input->forward && !Util::input->strafe && face == Direction::UP && *tower) {
			Util::player->aabb.offset(0, 1.05f, 0);
			me.motion.y = Util::player->motion.y = 0;
			//Util::player->onGround = true;
		}

		if (pseudoSlot != -1) {
			Util::player->setSelectedItemSlot(realSlot);
			Util::sendPacket(EquipPacket{realSlot});
		}
	});
}
