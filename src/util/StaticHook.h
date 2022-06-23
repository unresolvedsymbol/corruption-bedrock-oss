#pragma once

#include "Log.h"
#include "MCPELauncherAPI.h"
#include "util/MinecraftHandle.h"

static int hookHandle = 0;
static void *hookCache[
#ifdef SIZEHOOKS
		SIZEHOOKS
#else
	64
#endif
		];
static uintptr_t *scuffedOffset, realOffset, slideAddress;

struct RegisterStaticHook {
    RegisterStaticHook(const char *sym, void* hook, void** orig) {
		//Log(1) << "Self og result of " << sym << " was " << std::hex << dlsym(MinecraftHandle(), sym);
		//Log(1) << "Hooking \"" << sym << "\" to " << std::hex << hook;
		//*orig = dlsym(MinecraftHandle(), sym);
		//void *dead;
		void *hookPtr = mcpelauncher_hook2(MinecraftHandle(), static_cast<void *>(const_cast<char *>(sym)), hook, orig);
		//void *ret = mcpelauncher_hook((void *) sym, hook, orig);
		if (!hookPtr)
			Log(1) << "Unresolved symbol \"" << sym << "\".";
		else {
			if (!hookHandle) {
				scuffedOffset = reinterpret_cast<uintptr_t *>(orig);
				realOffset = reinterpret_cast<uintptr_t>(dlsym(MinecraftHandle(), sym));
			}
			hookCache[hookHandle++] = hookPtr;
		}
    }

    // workaround for a warning
    template <typename T>
    RegisterStaticHook(const char* sym, T hook, void** org) {
        union {
            T a;
            void* b;
        } hookUnion;
        hookUnion.a = hook;
        RegisterStaticHook(sym, hookUnion.b, org);
    }
};

#define _TInstanceHook(class_inh, pclass, iname, sym, ret, args...) \
struct _TInstanceHook_##iname class_inh { \
    static ret (_TInstanceHook_##iname::*_original)(args); \
    template <typename ...Params> \
    static ret original(pclass* _this, Params&&... params) { return (reinterpret_cast<_TInstanceHook_##iname *>(_this)->*_original)(std::forward<Params>(params)...); } \
    ret _hook(args); \
}; \
static RegisterStaticHook _TRInstanceHook_##iname (#sym, &_TInstanceHook_##iname::_hook, reinterpret_cast<void **>(&_TInstanceHook_##iname::_original)); \
ret (_TInstanceHook_##iname::*_TInstanceHook_##iname::_original)(args); \
ret _TInstanceHook_##iname::_hook(args)
#define _TInstanceDefHook(iname, sym, ret, type, args...) _TInstanceHook(: public type, type, iname, sym, ret, args)
#define _TInstanceNoDefHook(iname, sym, ret, args...) _TInstanceHook(, void, iname, sym, ret, args)

#define _TStaticHook(pclass, iname, sym, ret, args...) \
struct _TStaticHook_##iname pclass { \
    static ret (*_original)(args); \
    template <typename ...Params> \
    static ret original(Params&&... params) { return (*_original)(std::forward<Params>(params)...); } \
    static ret _hook(args); \
}; \
static RegisterStaticHook _TRStaticHook_##iname (#sym, &_TStaticHook_##iname::_hook, reinterpret_cast<void **>(&_TStaticHook_##iname::_original)); \
ret (*_TStaticHook_##iname::_original)(args); \
ret _TStaticHook_##iname::_hook(args)
#define _TStaticDefHook(iname, sym, ret, type, args...) _TStaticHook(: public type, iname, sym, ret, args)
#define _TStaticNoDefHook(iname, sym, ret, args...) _TStaticHook(, iname, sym, ret, args)
#define THook2(iname, ret, sym, args...) _TStaticNoDefHook(iname, sym, ret, args)
#define THook(ret, sym, args...) THook2(sym, ret, sym, args)
#define TClasslessInstanceHook2(iname, ret, sym, args...) _TInstanceNoDefHook(iname, sym, ret, args)
#define TClasslessInstanceHook(ret, sym, args...) TClasslessInstanceHook2(sym, ret, sym, args)
#define TInstanceHook2(iname, ret, sym, type, args...) _TInstanceDefHook(iname, sym, ret, type, args)
#define TInstanceHook(ret, sym, type, args...) TInstanceHook2(sym, ret, sym, type, args)
#define TStaticHook2(iname, ret, sym, type, args...) _TStaticDefHook(iname, sym, ret, type, args)
#define TStaticHook(ret, sym, type, args...) TStaticHook2(sym, ret, sym, type, args)
#define TVHook(ret, lib, parentSym, virtualSym, args...) \
char lol_##parentSym_##virtualSym = []() -> { \
    void **vtable = reinterpret_cast<void **>(dlsym(lib, parentSym)); \
    void *targetSym = dlsym(lib, virtualSym); \
    ret (*original = nullptr) (args); \
    ret (*hook = nullptr) (args); \
    for (void *vtEntry = vtable[0]; vtEntry != nullptr; ++vtEntry) \
	if (vtEntry == targetSym) { \
	    original = reinterpret_cast<ret (*) (args)>(vtEntry); \
	    vtEntry = &hook; \
	    break; \
	} \
    return 0; \
} \
