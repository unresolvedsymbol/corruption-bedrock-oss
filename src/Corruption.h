#pragma once

#include "module/ModuleManager.h"

#include "module/impl/world/Wallhack.h"
#include "module/impl/AntiCrash.h"
#include "module/impl/AntiCheat.h"

#include "module/impl/combat/AimBot.h"
#include "module/impl/combat/Aura.h"
#include "module/impl/combat/AutoHeal.h"
#include "module/impl/combat/Hitboxes.h"
#include "module/impl/combat/BowAim.h"

#include "module/impl/move/Glide.h"
#include "module/impl/move/NoFall.h"
#include "module/impl/move/Speed.h"
#include "module/impl/move/Step.h"
#include "module/impl/move/Velocity.h"
#include "module/impl/move/Sprint.h"
#include "module/impl/move/RideSpeed.h"

#include "module/impl/render/ESP.h"
#include "module/impl/render/HUD.h"

#include "module/impl/world/NoteBot.h"
#include "module/impl/world/Nuker.h"
#include "module/impl/world/SongStealer.h"
#include "module/impl/world/Phase.h"
#include "module/impl/world/Scaffold.h"

#include "module/impl/Keybinds.h"
#include "module/impl/FancyChat.h"

#include "module/impl/SimpleModules.h"
#include "module/impl/Commands.h"

using ModuleManagerImpl_t = ModuleManager<
		AimBot,
		Aura,
		//Criticals,
		Speed,
		AutoHeal,
		//ESP,
		Glide,
		Hitboxes,
		//HUD,
		Keybinds,
		NoFall,
		NoteBot,
		RideSpeed,
		SongStealer,
		Step,
		Velocity,
		List,
		Unload,
		PacketLogger,
		Particles,
		ItemSmith,
		JetPack,
		EggAura,
		Nuker,
		Sprint,
		Sneak,
		DeathCoords,
		Phase,
		WaterWalk,
		Effect,
		AddEnchant,
		VClip,
		HClip,
		FastFall,
		SetTimer,
		FastEat,
		ServerCrasher,
		FallBack,
		NoSlow,
		Teams,
		Name,
		AutoLog,
		Blink,
		Freecam,
		BowAim,
		NoShadow,
		NoHunger,
		FastClimb,
		Scaffold,
		PacketLoss,
		Zoom,
		Wallhack,
		GodMode,
		ElytraFly,
		Windows,
		AutoFish,
		AntiEffect,
		AntiCheat,
		AntiCrash,
		InventoryMove,
		AntiAim,
		FancyChat,
		HighJump,
		Derp
		/*Swim,
		SpoofElytra
		Say,
		Pot,
		EnchantSeed,
		Swim,
		SpoofElytra,
		Sniffer,
		MapExploit,
		AutoSell,*/
>;

/*
 * THE TWO BIG TODOS
 *
 * TAB UI
 * SAVING (WITHOUT SOME TRASH JSON LIB)
 */


struct Corruption {
	Corruption();

	~Corruption();

	static Corruption *get();

	ModuleManagerImpl_t &modules();

private:
	static Corruption *instance;

	ModuleManagerImpl_t moduleManager;
};
