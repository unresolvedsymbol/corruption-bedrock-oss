#include <util/serialize.h>
#include <network/protocol/MovePlayerPacket.h>
#include <network/protocol/InventoryTransactionPacket.h>
#include "Util.h"
#include "packet/AnonymousPacket.h"
#include "packet/EquipPacket.h"
#include "Corruption.h"

MinecraftGame *Util::game = nullptr;
Minecraft *Util::mc = nullptr;
ClientInstance *Util::client = nullptr;
LocalPlayer *Util::player = nullptr;
MoveInputHandler *Util::input = nullptr;

// Kill me
char Util::getColorFromString(std::string const &string) {
	unsigned char res = 0, x = 0;
	for (const char &c : string) {
		switch (c) {
			// \247 (color code symbol) == -62 -89
			case -62:
				if (x == 0) {
					x = 1;
					continue;
				}
			case -89:
				if (x == 1) {
					x = 2;
					continue;
				}
			// Ignore formats
			case 'l':
			case 'm':
			case 'o':
			case 'r':
				x = 0;
				continue;
			default:
				if (x == 2) {
					res = c;
					x = 0;
					continue;
				}
				x = 0;
		}
	}
	return res;
}

void Util::bgrToHsv(unsigned int value, float &h, float &s, float &v) {
	float r = (value & 0xFF) / 255;
	float g = (value >> 8 & 0xFF) / 255;
	float b = (value >> 16 & 0xFF) / 255;

	double min, max, delta;

	min = r < g ? r : g;
	min = min < b ? min : b;

	max = r > g ? r : g;
	max = max > b ? max : b;

	v = max; // v
	delta = max - min;
	if (delta < 0.00001) {
		s = 0;
		h = 0; // undefined, maybe nan?
		return;
	}
	if (max > 0.0) { // NOTE: if Max is == 0, this divide would cause a crash
		s = (delta / max); // s
	} else {
		// if max is 0, then r = g = b = 0
		// s = 0, h is undefined
		s = 0.0;
		h = NAN; // its now undefined
		return;
	}
	if (r >= max) // > is bogus, just keeps compilor happy
		h = (g - b) / delta; // between yellow & magenta
	else if (g >= max)
		h = 2.0 + (b - r) / delta; // between cyan & yellow
	else
		h = 4.0 + (r - g) / delta; // between magenta & cyan

	h *= 60.f; // degrees

	if (h < 0.0)
		h += 360.f;
}

int Util::hsvToBgr(unsigned char h, unsigned char s, unsigned char v) {
	unsigned char r, g, b;
	unsigned char region, remainder, p, q, t;

	if (s == 0) {
		r = v;
		g = v;
		b = v;
		return r | g << 8 | b << 16;
	}

	region = h / 43;
	remainder = (h - (region * 43)) * 6;

	p = (v * (255 - s)) >> 8;
	q = (v * (255 - ((s * remainder) >> 8))) >> 8;
	t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

	switch (region) {
		case 0:
			r = v; g = t; b = p;
			break;
		case 1:
			r = q; g = v; b = p;
			break;
		case 2:
			r = p; g = v; b = t;
			break;
		case 3:
			r = p; g = q; b = v;
			break;
		case 4:
			r = t; g = p; b = v;
			break;
		default:
			r = v; g = p; b = q;
			break;
	}

	return r | g << 8 | b << 16;
}

bool Util::isTeamed(const Actor &other) {
 	return getColorFromString(player->getNameTag()) == getColorFromString(other.getNameTag());
}

void Util::sendPacket(const Packet &packet, bool event) {
	if (event)
		client->getPacketSender()->send(const_cast<Packet &>(packet));
	else
		client->getPacketSender()->sendToServer(const_cast<Packet &>(packet));
}

bool Util::parseRest(args_t args, int index, std::string &result) {
	std::ostringstream ss;
	std::copy(args.begin() + index, args.end(), std::ostream_iterator<std::string>(ss, " "));
	result = ss.str();
	return !result.empty();

	/*for (; index < args.size(); ++index) {
		result += std::istringstream{args[index]}.str();
		if (index != args.size() - 1)
			result += " "; // Please kill me now
	}
	return result.size();*/
}

bool Util::valid(const Actor &actor, bool checkTeam, float range, float yaw, float pitch) {
	if (actor.getRuntimeID() == player->getRuntimeID()
		//|| actor.ticksExisted < 400
		|| (range > 0 && actor.distanceTo(*player) > range)
		|| actor.getNameTag().size() < 3
		|| actor.width < .49f/*.57f*/ || actor.width > .85f // Hyperlands shrinks hitboxes damn
		|| actor.height < 1.75f || actor.height > 1.85f
		|| (checkTeam && Corruption::get()->modules().get<Teams>().get() && isTeamed(actor)))
		return false;

	auto rotDiff = player->getRotation().distance(wrap(getRotationsTo(actor)));

	if (rotDiff.yaw > yaw || rotDiff.pitch > pitch)
		return false;

	/*for (auto entry : player->getLevel().getPlayerList())
		if (player.getUniqueID().data == entry.second.uid.data)
			return true;

	return false;*/

	return true;
}

Vec2 Util::wrap(const Vec2 &angle) {
	Vec2 newAngle = angle;

	while (newAngle.x > 90.f)
		newAngle.x -= 180.f;
	while (newAngle.x < -90.f)
		newAngle.x += 180.f;

	while (newAngle.y > 180.f)
		newAngle.y -= 360.f;
	while (newAngle.y < -180.f)
		newAngle.y += 360.f;

	return newAngle;
}

float Util::getBaseMoveSpeed() {
	float value = .2873f;
	if (const MobEffectInstance *speed = player->getEffect(*MobEffect::MOVEMENT_SPEED))
		value *= 1 + .2 * (speed->getAmplifier() + 1);
	return value;
}

/*Vec3 Util::getLookVec(const Vec2 &angle) {
	return {
		-std::sin(angle.yaw) * std::cos(angle.pitch),
		-std::sin(angle.pitch),
		std::cos(angle.yaw) * std::cos(angle.pitch)
	};
}*/

Vec3 Util::getLookVec(const Vec2 &angle) {
	float rads = M_PI / 180;
	float pitch = angle.pitch * rads,
		yaw = (angle.yaw + 90) * rads;
	return {
		cosf(pitch) * cosf(yaw),
		-sinf(pitch),
		cosf(pitch) * sinf(yaw)
	};
}

Vec2 Util::getMoveVec(float speed, bool input, float direction) {
	direction += 90;
	char strafe = 0, forward = 1;
	if (input) {
		forward = Util::input->forward > 0.f ? 1 : Util::input->forward < 0.f ? -1 : 0;
		strafe = Util::input->strafe > 0.f ? 1 : Util::input->strafe < 0.f ? -1 : 0;

		if (forward && strafe) {
			direction -= strafe * forward * 45;
			strafe = 0;
		}
	}
	float mX = cosf(direction * M_PI / 180),
			mZ = sinf(direction * M_PI / 180);
	return { forward * speed * mX + strafe * speed * mZ, forward * speed * mZ - strafe * speed * mX };
}

Vec2 Util::getRotationsTo(const Actor &target) {
	return getRotationsTo(target.getPos().subtract(0, .46f /* Body */, 0));
}

Vec2 Util::getRotationsTo(const Vec3 &pos, const Vec3 &from) {
	static const float deg = 180 / M_PI; // Radians to degrees
	Vec3 diff = pos.subtract(from); // Difference
	float dist = diff.delta(); // Distance for pitch calculation
	return {
		-(std::atan2(diff.y, dist) * deg), // Pitch
		std::atan2(diff.z, diff.x) * deg - 90 // Yaw
	};
}

Vec2 *Util::project(const Vec3 &pos, float width, float height, bool tracers) {
	auto rend = client->getLevelRenderer();
	if (!rend)
		return nullptr;
	auto r = (*rend->worldViewMat) * (*rend->identMat) * (*rend->projMat) * Vec4{pos.x, pos.y, pos.z, 1.f};
	if (r.w < .01f && !tracers)
		return nullptr;
	return new Vec2{(width / 2.f) + (.5f * (r.x / r.w) * width + .5f),
			(height / 2.f) - (.5f * (r.y / r.w) * height + .5f)};
}

AABB *Util::project(const AABB &aabb, float width, float height) {
	float left = FLT_MAX, right = FLT_MIN, top = FLT_MAX, bottom = FLT_MIN;

	for (auto vertex : {
			Vec3{aabb.min.x, aabb.min.y, aabb.min.z},
			Vec3{aabb.min.x, aabb.min.y, aabb.max.z},
			Vec3{aabb.max.x, aabb.min.y, aabb.min.z},
			Vec3{aabb.max.x, aabb.min.y, aabb.max.z},

			Vec3{aabb.min.x, aabb.max.y, aabb.min.z},
			Vec3{aabb.min.x, aabb.max.y, aabb.max.z},
			Vec3{aabb.max.x, aabb.max.y, aabb.min.z},
			Vec3{aabb.max.x, aabb.max.y, aabb.max.z}
	}) {
		auto proj = Util::project(vertex, width, height);

		if (!proj)
			continue;

		left = std::min<float>(left, proj->x);
		right = std::max<float>(right, proj->x);
		top = std::min<float>(top, proj->y);
		bottom = std::max<float>(bottom, proj->y);
	}

	if (left == FLT_MAX)
		return nullptr;

	return new AABB{left, bottom, 1, right, top, 1};
}

void Util::swap(short oldSlot, short slot) {
	const ContainerItemStack *stack = player->getInventoryMenu().getSlot(slot), *oldStack = player->getInventoryMenu().getSlot(oldSlot);

	// Pickup new
	sendPacket(AnonymousPacket{"InventoryTransactionPacket", 0x1e, [&](BinaryStream &stream) {
		stream.writeUnsignedVarInt(0); // transaction type
		stream.writeUnsignedVarInt(2); // transaction count
		// begin action 1
		stream.writeUnsignedVarInt(0); // source type
		stream.writeVarInt(124); // window id
		stream.writeUnsignedVarInt(0); // inventory slot
		// begin custom itemstack (oldItem)
		stream.writeVarInt(0); // id, don't write anything else if empty
		// begin custom itemstack (newItem)
		serialize<ItemStack>::write(*stack, stream);
		// end action 1
		// begin action 2
		stream.writeUnsignedVarInt(0); // source type
		stream.writeVarInt(0); // window id
		stream.writeUnsignedVarInt(slot); // inventory slot
		// begin custom itemstack (oldItem)
		serialize<ItemStack>::write(*stack, stream);
		// begin custom itemstack (newItem)
		stream.writeVarInt(0); // id
		// end action 2
	}});

	// Place on old
	sendPacket(AnonymousPacket{"InventoryTransactionPacket", 0x1e, [&](BinaryStream &stream) {
		stream.writeUnsignedVarInt(0); // transaction type
		stream.writeUnsignedVarInt(2); // transaction count
		// begin action 1
		stream.writeUnsignedVarInt(0); // source type
		stream.writeVarInt(124); // window id
		stream.writeUnsignedVarInt(0); // inventory slot
		// begin custom itemstack (oldItem)
		serialize<ItemStack>::write(*stack, stream);
		// begin custom itemstack (newItem)
		serialize<ItemStack>::write(*oldStack, stream); // id, don't write anything else if empty
		// end action 1
		// begin action 2
		stream.writeUnsignedVarInt(0); // source type
		stream.writeVarInt(0); // window id
		stream.writeUnsignedVarInt(oldSlot); // inventory slot
		// begin custom itemstack (oldItem)
		serialize<ItemStack>::write(*oldStack, stream);
		// begin custom itemstack (newItem)
		serialize<ItemStack>::write(*stack, stream); // id
		// end action 2
	}});

	// Send equipment packet if in hotbar
	if (oldSlot < 9)
		sendPacket(EquipPacket{oldSlot, oldStack});

	// Put old where new was
	sendPacket(AnonymousPacket{"InventoryTransactionPacket", 0x1e, [&](BinaryStream &stream) {
		stream.writeUnsignedVarInt(0); // transaction type
		stream.writeUnsignedVarInt(2); // transaction count
		// begin action 1
		stream.writeUnsignedVarInt(0); // source type
		stream.writeVarInt(124); // window id
		stream.writeUnsignedVarInt(0); // inventory slot
		// begin custom itemstack (oldItem)
		serialize<ItemStack>::write(*oldStack, stream); // id
		// begin custom itemstack (newItem)
		stream.writeVarInt(0);
		// end action 1
		// begin action 2
		stream.writeUnsignedVarInt(0); // source type
		stream.writeVarInt(0); // window id
		stream.writeUnsignedVarInt(slot); // inventory slot
		// begin custom itemstack (oldItem)
		stream.writeVarInt(0);
		// begin custom itemstack (newItem)
		serialize<ItemStack>::write(*oldStack, stream); // id
		// end action 2
	}});
}

// Fakes the item being moved from your 2x2 grid to an empty inventory slot (For Vanilla/Realms/BDS)
void Util::gridToNew(ItemStack &stack) {
	for (short slot = 0; slot < 36; slot++) {
		auto hackstack = Util::player->getInventoryMenu().getSlot(slot);
		if (hackstack && hackstack->isNull()) {
			Util::sendPacket(AnonymousPacket{"InventoryTransactionPacket", 0x1e, [&](BinaryStream &stream) {
				stream.writeUnsignedVarInt(0);
				stream.writeUnsignedVarInt(2);
				// action
				stream.writeUnsignedVarInt(0); // type
				stream.writeVarInt(0); // window
				stream.writeUnsignedVarInt(slot); // slot
				stream.writeVarInt(0); // old item air
				serialize<ItemStack>::write(stack, stream); // new ench
				// second action
				stream.writeUnsignedVarInt(100); // type
				stream.writeVarInt(-2); // window
				stream.writeUnsignedVarInt(2); // slot
				serialize<ItemStack>::write(stack, stream); // old ench
				stream.writeVarInt(0); // new item air
			}}, true);
			break;
		}
	}
}

// Fakes the item being placed over from your inventory grid (Mineplex enchanting etc)
void Util::gridToHeld(ItemStack &stack) {
	Util::sendPacket(AnonymousPacket{"InventoryTransactionPacket", 0x1e, [&](BinaryStream &stream) {
		stream.writeUnsignedVarInt(0);
		stream.writeUnsignedVarInt(2);
		// action
		stream.writeUnsignedVarInt(0); // type
		stream.writeVarInt(0); // window
		stream.writeUnsignedVarInt(Util::player->getSelectedItemSlot()); // slot
		stream.writeVarInt(0); // old item air
		serialize<ItemStack>::write(stack, stream); // new ench
		// second action
		stream.writeUnsignedVarInt(100); // type
		stream.writeVarInt(-2); // window
		stream.writeUnsignedVarInt(2); // slot
		serialize<ItemStack>::write(stack, stream); // old ench
		stream.writeVarInt(0); // new item air
	}}, true);
}

Vec3 Util::stepTeleport(Vec3 origin, Vec3 target, Vec2 speed, float distance) {
	if (speed.x == 0) {
		Util::sendPacket(MovePlayerPacket{Util::player->getRuntimeID(), Util::player->onGround, target, Vec3{Util::getRotationsTo(target, origin)}}, true);
		return target;
	}

	auto rots = Vec3{Util::getRotationsTo(target, origin)};
	char offsetCounter = 0;
	float clipDistance = 0;

	while (origin.subtract(target).delta() > std::max(speed.x, distance)) {
		rots = Vec3{Util::getRotationsTo(target, origin)};
		auto move = Util::getMoveVec(speed.x, false, rots.y);

		origin.x += move.x;
		origin.z += move.y;

		// Move vertically and horizontal
		if (std::abs(origin.y - target.y) > speed.y) {
			auto dY = origin.y < target.y ? speed.y : origin.y > target.y ? -speed.y : 0;
			if (Util::player->getRegion().getMaterial(origin.add(0, dY, 0)).isReplaceable())
				origin.y += dY;
		}

		// Speed hack thing
		/*if (offsetCounter > 2) {
			origin.y += .4;
			offsetCounter = 0;
		} else offsetCounter++;*/

		//if (Util::player->getRegion().getMaterial(origin).isReplaceable() || (clipDistance += speed.x) > 1) {
		Util::sendPacket(MovePlayerPacket{Util::player->getRuntimeID(), Util::player->onGround, origin, rots},true);
		//	clipDistance = 0;
		//}
	}

	// Vec3::delta() is horizontal distance so finish veritcal change if needed
	while (std::abs(origin.y - target.y) > 1) {
		origin.y += origin.y < target.y ? .5f : origin.y > target.y ? -.5f : 0;

		rots = Vec3{Util::getRotationsTo(target, origin)};

		//if (Util::player->getRegion().getMaterial(sPos).isReplaceable())
		Util::sendPacket(MovePlayerPacket{Util::player->getRuntimeID(), Util::player->onGround, origin, rots}, true);
	}

	return origin;
}

std::string Util::stripColors(const std::string &coloredString) {
	static const std::regex colorCodes{"\u00A7[0-9A-Ga-gK-Ok-oRr]"};
	return std::regex_replace(coloredString, colorCodes, "");
}

bool FOVComparator::operator()(Player *a, Player *b) {
	auto aC = Util::player->getRotation().distance(Util::getRotationsTo(*a)), bC = Util::player->getRotation().distance(Util::getRotationsTo(*b)); // change
	float aD = aC.x + aC.y, bD = bC.x + bC.y; // delta
	// those probably mean the same thing
	return aD < bD;
}

bool DistanceComparator::operator()(Player *a, Player *b) {
	return Util::player->distanceTo(*a) < Util::player->distanceTo(*b);
}

void Matrix4::operator=(const Matrix4 &o) {
	for (char i = 0; i < 4; ++i)
		for (char j = 0; j < 4; j++)
			m[i][j] = o.m[i][j];
}

Vec4 Matrix4::operator*(const Vec4 &o) const {
	Vec4 res{};
	for (char i = 0; i < 4; ++i)
		for (char j = 0; j < 4; j++)
			res.m[i] += m[j][i] * o.m[j];
	return res;
}

Matrix4 Matrix4::operator*(const Matrix4 &o) const {
	Matrix4 res{};
	for (char i = 0; i < 4; ++i)
		for (char j = 0; j < 4; ++j)
			for (char k = 0; k < 4; ++k)
				res.m[i][j] += m[i][k] * o.m[k][j];
	return res;
}
