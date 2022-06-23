#include "HUD.h"
#include "Corruption.h"
#include "events/impl/RenderEvents.h"

HUD::HUD() : ToggleModule("HUD", true) {
	addDynamicListener<ExtRenderEvent>([&](auto &re) {
		std::vector<Module*> cache;

		std::copy_if(Corruption::get()->modules().begin(), Corruption::get()->modules().end(), std::back_inserter(cache), [](auto *m) {
			return m->isToggleable() && static_cast<ToggleModule *>(m)->get();
		});

		std::sort(cache.begin(), cache.end(), [](auto l, auto r) {
			return ImGui::CalcTextSize(l->getName().c_str()).x > ImGui::CalcTextSize(r->getName().c_str()).x;
		});

		float y = re.height;
		char temp = hue;
		for (auto cached : cache) {
			const char *second = cached->getName().c_str();
			auto size = ImGui::CalcTextSize(second);
			y -= (size.y + 3);
			re.drawList->AddRectFilled({re.width - size.x - 10, y}, {re.width, y}, 0xC0000000);
			re.drawList->AddText({re.width - size.x - 4, y + 1.5f},
								 0xFF000000 | Util::hsvToBgr(temp += *moduleSpeed, *saturation, *value), second);
		}

		hue += *speed;
	});
}