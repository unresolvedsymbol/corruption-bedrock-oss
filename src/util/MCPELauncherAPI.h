#pragma once

#ifdef __cplusplus
extern "C" {
#endif

__DEPRECATED_IN(0)
void *mcpelauncher_hook(void *symbol, void *hook, void *original);

void *mcpelauncher_hook2(void *lib, void *sym, void *hook, void *orig);
void mcpelauncher_hook2_add_library(void *lib);
void mcpelauncher_hook2_remove_library(void *lib);
void mcpelauncher_hook2_delete(void *lib);
void mcpelauncher_hook2_apply();

#ifdef __cplusplus
}
#endif

// Accessor lol
struct HookManager {
	struct HookInstance {
		char pad[0x20];
		uintptr_t *orig;
	};
};