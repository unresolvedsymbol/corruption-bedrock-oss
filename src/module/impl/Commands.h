#pragma once

#include "module/Module.h"
#include "module/ToggleModule.h"

#define Add(name) struct name : Module {\
    name();\
};

Add(List)
Add(Unload)
Add(TestEvents)
Add(Say)
Add(VClip)
Add(HClip)
Add(Pot)
Add(Dupe)
Add(Name)
Add(AddEnchant)
Add(Teleport)
Add(SetTimer)
Add(Effect)
Add(Damage)
Add(EnchantSeed)

#undef Add