#include "RideSpeed.h"
#include "events/impl/MiscEvents.h"

RideSpeed::RideSpeed() : ToggleModule("RideControl") {
	addDynamicListener<RideTick>([&](auto &e) {
		if (*control)
			e.ride.rotation.yaw = Util::player->rotation.yaw;

		if (*fly)
			e.ride.motion.y = Util::input->jump ? *vertical : (Util::input->sneak ? -*vertical : *glide);

		if (Util::input->forward || Util::input->strafe) {
			if (*absolute) {
				auto move = Util::getMoveVec(*speed, true, e.ride.rotation.yaw);
				e.ride.motion.x = move.x;
				e.ride.motion.z = move.y;
			} else {
				e.ride.motion.x *= *factor;
				e.ride.motion.z *= *factor;
			}
		} else {
			e.ride.motion.x = e.ride.motion.z = 0;
		}
	});
}