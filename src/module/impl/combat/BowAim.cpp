#include "BowAim.h"
#include "events/impl/MiscEvents.h"

BowAim::BowAim() : ToggleModule("BowAim") {
	addDynamicListener<MoveEvent>([&](auto &me) {
		if (Util::player->getSelectedItem().getId() == 261 && Util::player->isUsingItem()) {
			Util::forValidPlayers([&](auto p) {
				if (Util::valid(p, true, 60, 30) && Util::player->canSee(p)) {
					auto ourPos = Util::player->pos.add(me.motion.multiply(1, 0));

					int d = Util::player->distanceTo(p);
					d -= d % 2;

					// TODO: Improve prediction
					// multiplier of 12 if sprinting faster than .25? doesn't seen to work on .35+ / .5
					// walking was 6 idk

					auto theirPos = p.pos.add(p.pos.subtract(p.prevPos).multiply(*distance ? (d / 2) : 1, 1).multiply(
							p.isSprinting() ? *sprintFactor : (p.isUsingItem() ? *usingFactor : *normalFactor), 1));
					//multiply(d / 2, 1).multiply(p.isSprinting() ? 1.25 : 1, 1)

					float vel = Util::player->getTicksUsingItem() / 20;
					vel = (vel * vel + vel * 2) / 3;

					Log() << p.pos.subtract(p.prevPos).delta() << " "
						  << (p.isSprinting() ? "sprinting" : (p.isSneaking() ? "sneaking" : (p.isUsingItem() ? "using"
																											  : "walking")));

					auto rots = Util::wrap(Util::getRotationsTo(theirPos, ourPos));
					if (vel > 0.1) {
						if (vel > 1)
							vel = 1;
						Log() << vel;
						rots.pitch = calcTraj(theirPos.subtract(ourPos).delta(),
											  theirPos.y - .45f /* torso ish */ - ourPos.y, vel);
					}
					Util::player->setRot(rots);

					return false;
				}
				return true;
			});
		}
	});
}

float BowAim::calcTraj(float horiz, float vert, float vel) {
	float g = *gravity; //0.006f;//000000052154064;
	vel *= vel;
	return -((180 / static_cast<float>(M_PI)) * std::atan((vel - std::sqrt(vel * vel - g * (g * (horiz * horiz) + (2 * vert) * vel))) / (g * horiz)));
}
