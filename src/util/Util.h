#pragma once

#include "client/MinecraftGame.h"
#include "client/ClientInstance.h"

#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <numeric>
#include <iterator>
#include <sstream>
#include <optional>
#include <set>

#include "module/Module.h"

struct Util {
	static Minecraft *mc;
	static MinecraftGame *game;
	static ClientInstance *client;
	static MoveInputHandler *input;
	static LocalPlayer *player;

	template<typename T>
	static void forValidPlayers(T fn) {
		player->getLevel().forEachPlayer([&](Player &player) {
			return !(Util::valid(player) && !fn(player));
		});
	}

	template<typename Consumer, typename Predicate, typename Comparator>
	static bool forEachPlayerSortedIf(Consumer consumer, Predicate predicate, Comparator comparator) {
		// Better? Does it even work?
		std::set<Player *, Comparator> copy;
		for (auto p : player->getLevel().players)
			if (predicate(*p))
				copy.insert(p);
		/*player->getLevel().forEachPlayer([&](Player &player) {
			if (predicate(player))
				copy.insert(&player);
			return true;
		});*/
		//std::copy_if(player->getLevel().players.begin(), player->getLevel().players.end(), copy.begin(), predicate);
		for (auto p : copy)
			if (!consumer(*p))
				break;
		return !copy.empty();

		// This is inefficient, but for now...
		/*std::vector<Player*> copy;
		//copy.reserve() level players size ???
		player->getLevel().forEachPlayer([&](Player &player) {
			if (predicate(player))
				copy.emplace_back(&player);
			return true;
		});
		std::sort(copy.begin(), copy.end(), comparator);
		for (unsigned int i = 0; i < copy.size(); ++i)
			if (!consumer(*copy[i]))
				break;
		return !copy.empty();*/
	}

	static char getColorFromString(std::string const &string);

	static void bgrToHsv(unsigned int value, float &h, float &s, float &v);

	static int hsvToBgr(unsigned char h, unsigned char s, unsigned char v);

	static bool isTeamed(const Actor &other);

	static void sendPacket(const Packet &packet, bool event = true);

	template<typename T>
	static bool parse(args_t args, int index, T &result) {
		return index < args.size() && std::istringstream(args[index]) >> result;
	}

	template<typename T>
	static bool parse(args_t args, int index, T &result, std::ios::fmtflags flags) {
		if (index >= args.size())
			return false;
		std::stringstream ss;
		ss << flags << args[index];
		return ss >> result;
		/*static std::stringstream ss;
		static auto ogFlags = ss.flags();
		ss.str(std::string(""));
		ss.clear();
		ss.setf(flags);
		ss << args[index];
		ss.flags(flags);
		return ss >> result;*/
	}

	static bool parseRest(args_t args, int index, std::string &result);

	static bool valid(const Actor &actor, bool checkTeam = false, float range = 0, float yaw = 360, float pitch = 180);

	static float getBaseMoveSpeed();

	static Vec3 getLookVec(const Vec2 &angle);

	static Vec2 getMoveVec(float speed, bool input = true, float direction = Util::player->getRotation().yaw);

	// Rotations to body by default
	static Vec2 getRotationsTo(const Actor &target);

	static Vec2 getRotationsTo(const Vec3 &pos, const Vec3 &from = Util::player->getPos());

	static Vec2 *project(const Vec3 &pos, float width, float height, bool tracers = false);

	static AABB *project(const AABB &bb, float width, float height);

	static Vec2 wrap(const Vec2 &angle);

	static void swap(short oldSlot, short slot);

	static void gridToNew(ItemStack &);

	static void gridToHeld(ItemStack &);

	// Origin position, Target position, Step speed (H, V), Minimum distance to target before satisfied
	// Returns new server position
	static Vec3 stepTeleport(Vec3 origin, Vec3 target, Vec2 speed, float distance);

	// This is mineman implemented
	static mce::UUID generateRandomId(int offset);

	static std::string stripColors(const std::string &);

	/*template<typename T>
	static std::string toString(T t) {
		std::stringstream ss;
		ss << t;
		return ss.str();
	};*/
};

struct Vec4 {
	union {
		struct {
			float x, y, z, w;
		};
		struct {
			Vec2 min, max;
		};
		float m[4];
	};

	/*friend std::ostream& operator<<(std::ostream &ss, Vec4 &vec4) {
		return ss << "{" << vec4.x << ", " << vec4.y << ", " << vec4.z << ", " << vec4.w << "}";
	}*/
};

struct Matrix4 {
	float m[4][4];

	/*inline float& operator()(char row, char column) {
		return m[row][column];
	}*/

	Matrix4 operator*(const Matrix4& o) const;

	Vec4 operator*(const Vec4& o) const;

	void operator=(const Matrix4& o);

	/*friend std::ostream& operator<<(std::ostream &ss, Matrix4& m) {
		return ss << "mat{" << m.m[0] << " " << m.m[1] << " " << m.m[2] << "}";
	}*/
};

struct FOVComparator {
	bool operator()(Player *a, Player *b);
};

struct DistanceComparator {
	bool operator()(Player *a, Player *b);
};

struct LevelRenderer {
	// I think this is the MatrixStackRef
	char pad_0000[0x42c];
	Matrix4 *worldViewMat; // 0x42c
	char pad_042c[0xc];
	Matrix4 *identMat; // 0x43c
	char pad_043c[0xc];
	Matrix4 *projMat; // 0x44c

	LevelRendererPlayer &getLevelRendererPlayer();

	void onAppSuspended();
	void onAppResumed();
};
