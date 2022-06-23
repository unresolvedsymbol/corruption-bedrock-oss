#pragma once

#include "imgui.h"

struct RenderCameraEvent {
	// Main ingredient for game builtin render functions
	BaseActorRenderContext &ctx;
};

struct ExtRenderEvent {
	ImFont *font;
	ImDrawList *drawList;
	const float width, height;
};