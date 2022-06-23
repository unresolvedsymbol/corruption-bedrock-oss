#include "PatchUtils.h"
#include <dlfcn.h>

void *PatchUtils::patternSearch(void *handle, const char *pattern) {
    std::vector<unsigned char> patternRaw;
    std::vector<unsigned char> patternMask;
    while (pattern[0] && pattern[1]) {
        if (*pattern == ' ') {
            ++pattern;
            continue;
        }
        if (pattern[0] == '?' && pattern[1] == '?') {
            patternRaw.push_back(0);
            patternMask.push_back(0);
        } else {
            patternRaw.push_back(static_cast<char>(std::strtoul(pattern, nullptr, 16)));
            patternMask.push_back(0xFF);
        }
        pattern += 2;
    }

    size_t base, size;
    // TODO: Pattern search linker func
    //get_library_code_region(handle, base, size);
    for (size_t i = size - patternRaw.size(); i > 0; --i) {
        for (size_t j = 0; j < patternRaw.size(); j++) {
            if (patternRaw[j] != (((unsigned char *) base)[i + j] & patternMask[j]))
                goto skip;
        }
        return &reinterpret_cast<char *>(base)[i];
        skip: ;
    }
    return nullptr;
}

void PatchUtils::patchCallInstruction(void* patchOff, void* func, bool jump) {
    unsigned char* data = static_cast<unsigned char *>(patchOff);
#ifdef __arm__
    if (!jump)
      throw std::runtime_error("Non-jump patches not supported in ARM mode");
    bool thumb = ((size_t) patchOff) & 1;
    if (thumb)
      data--;
    //Log::trace(TAG, "Patching - original: %i %i %i %i %i", data[0], data[1], data[2], data[3], data[4]);
    if (thumb) {
        unsigned char patch[4] = {0xdf, 0xf8, 0x00, 0xf0};
        memcpy(data, patch, 4);
    } else {
        unsigned char patch[4] = {0x04, 0xf0, 0x1f, 0xe5};
        memcpy(data, patch, 4);
    }
    memcpy(&data[4], &func, sizeof(int));
    //Log::trace(TAG, "Patching - result: %i %i %i %i %i", data[0], data[1], data[2], data[3], data[4]);
#else
    //Log::trace(TAG, "Patching - original: %i %i %i %i %i", data[0], data[1], data[2], data[3], data[4]);
    data[0] = static_cast<unsigned char>(jump ? 0xe9 : 0xe8);
    intptr_t ptr = ((reinterpret_cast<intptr_t>(func)) - reinterpret_cast<intptr_t>(patchOff) - 5);
    if (ptr > INT_MAX || ptr < INT_MIN)
        throw std::runtime_error("patchCallInstruction: out of range");
    int iptr = ptr;
    memcpy(&data[1], &iptr, sizeof(int));
    //Log::trace(TAG, "Patching - result: %i %i %i %i %i", data[0], data[1], data[2], data[3], data[4]);
#endif
}

void PatchUtils::VtableReplaceHelper::replace(const char* name, void* replacement) {
    replace(dlsym(lib, name), replacement);
}

void PatchUtils::VtableReplaceHelper::replace(void* sym, void* replacement) {
    for (int i = 0; ; i++) {
        if (referenceVtable[i] == nullptr)
            break;
        if (referenceVtable[i] == sym) {
            vtable[i] = replacement;
            return;
        }
    }
}

void PatchUtils::VtableReplaceHelper::replaceOrig(const char *name, void *replacement, void **orig) {
	for (int i = 0; ; i++) {
		if (referenceVtable[i] == nullptr)
			break;
		if (referenceVtable[i] == dlsym(lib, name)) {
			*orig = vtable[i];
			vtable[i] = replacement;
			return;
		}
	}
}

size_t PatchUtils::getVtableSize(void** vtable) {
    for (size_t size = 2; ; size++) {
        if (vtable[size] == nullptr)
            return size;
    }
}

void PatchUtils::dumpVtable(void **vtable) {
	Log(1) << "Dumping vtable...";
	for (int i = 0; ; i++) {
		void *currentFunc = vtable[i];
		if (!currentFunc)
			break;
		Dl_info info;
		int status = dladdr(currentFunc, &info);
		const char *sym = info.dli_sname;
		Log(1) << "[" << i << "] " << sym;
	}
	Log(1) << "==================";
}
