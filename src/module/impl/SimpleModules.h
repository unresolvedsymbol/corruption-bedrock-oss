#pragma once

#include "../Module.h"
#include "../ToggleModule.h"

#define Add(name) struct name : ToggleModule { \
    name(); \
};

// These are smaller modules that are not changed often. More complex ones are split into their own object for compile speed.

Add(PacketLogger)
Add(JetPack)
Add(EggAura)
Add(WaterWalk)
Add(Dolphin)
Add(FastFall)
Add(AutoTotem)
Add(NoHunger)
Add(NoPacket)
Add(FastEat)
Add(ServerCrasher)
Add(AutoLog)
Add(PacketLoss)
Add(InstantBreak)
Add(FastClimb)
Add(ElytraFly)
Add(GodMode)
Add(NoSlow)
Add(Sneak)
Add(Teams)
Add(SpoofElytra)
Add(Swim)
Add(NoShadow)
Add(FallBack)
Add(SafeWalk)
Add(Windows)
Add(AutoSell)
Add(DeathCoords)
Add(AntiEffect)
Add(Zoom)
Add(InventoryMove)
Add(Sniffer)
Add(AntiAim)
Add(HighJump)
Add(Derp)

struct ItemSmith : ToggleModule {
	std::map<unsigned char, unsigned short> enchants;
	// Zero = unmodified (except damage)
	short id = 0;
	unsigned char count = 0, data = 0;

	ItemSmith();
};
struct Particles : ToggleModule {
	Particles();

	Value<int> item{this, "Item", 116}, // Enchantment table looks cool, no item aux data for wools :(
	// 467 heart of the sea
	// 513 sheild - has no texture but still sound
	// 426 end crystal
	// 438 splash potion, slurping no texture
	// 385 fireball
	// 347 and 345 compass/clock - but renders like fucking confetti????
	// 246 good old glowing obby
		multiplier{this, "Multiplier", 20},
		delay{this, "Delay", 0};
};
struct AutoFish : ToggleModule {
	AutoFish();

private:
	unsigned char delay, stage;
};
struct Blink : ToggleModule {
	Blink();

private:
	std::vector<std::tuple<bool, Vec3, Vec3>> moves;
};
struct Freecam : ToggleModule {
	Freecam();

private:
	Vec3 oldPosition;
};

#undef Add