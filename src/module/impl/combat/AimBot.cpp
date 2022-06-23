#include "AimBot.h"

// :c
static Vec2* getCurve(Vec2 p0, Vec2 c1, Vec2 c2, Vec2 p1, int segments) {
	Vec2 *points = static_cast<Vec2 *>(malloc(segments * sizeof(Vec2)));
	float t_step = 1.f / segments;
	for (int i_step = 1; i_step <= segments; ++i_step) {
		float t = t_step * i_step;
		float u = 1.f - t;
		float w1 = u * u * u;
		float w2 = 3 * u * u * t;
		float w3 = 3 * u * t * t;
		float w4 = t * t * t;
		points[i_step] = {
				w1 * p0.x + w2 * p1.x + w3 * c2.x + w4 * c1.x,
				w1 * p0.y + w2 * p1.y + w3 * c2.y + w4 * c1.y
		};
	}
	return points;
};

AimBot::AimBot() : ToggleModule("AimAssist") {
	// TODO: Figure out why it clips past the target sometimes? Is the rotations calculator failing?
	// TODO: Fix angle wrapping so above doesn't occur if I'm correct
	// TODO: Fix pitch not like it's important
		addDynamicListener<PlayerTick>([&](auto &) {
			if (!target || target->isRemoved() || !Util::valid(*target, true, *range, *fov)) {
				target = nullptr;
				Util::forValidPlayers([&](auto &p) {
					if (!Util::valid(p, true, *range, *fov))
						return true;
					target = &p;
					targetWidth = p.width;
					targetHeight = p.height;
					return false;
				});
			} else {
				// BREAD MAN COPY RIGHT
				if (randDelay < *randSpeed)
					++randDelay;
				else {
					randWidth1 = targetWidth / *width * rd(re) - targetWidth / (*width * 2.f);
					randWidth2 = targetWidth / *width * rd(re) - targetWidth / (*width * 2.f);
					randHeight = targetHeight / *height * rd(re) - targetHeight / (*height * 2.f);
					randDelay = 0;
				}

				// Besides this lol I'm big brain
				auto delta = Util::wrap(Util::getRotationsTo(target->getInterpolatedPosition(3).add(randWidth1, randHeight - 2.2f + (target->height * .8f), randWidth2)).subtract(Util::player->getRotation()));
				float absPitch = std::abs(delta.x), absYaw = std::abs(delta.y);
				if ((absYaw + absPitch) > *min) {
					auto limit = delta.limit({absPitch / *speed + *base, absYaw / *speed + *base});
					Util::player->_applyTurnDelta({limit.x, limit.y});
				}
			}
		});
}