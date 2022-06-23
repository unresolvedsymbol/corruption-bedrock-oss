#include <network/protocol/RemoveActorPacket.h>
#include <network/protocol/SetActorDataPacket.h>
#include <network/protocol/AddPlayerPacket.h>
#include <imgui.h>
#include <imgui_internal.h>
#include "ESP.h"
#include "events/impl/RenderEvents.h"
#include "events/impl/PacketEvents.h"

ESP::ESP() : ToggleModule("ESP") {
	// Potion tags by Void
	// Direkt private had an extremely similar cache, in fact the code was
	// so big that they decided to put it in it's own PotionTags.java

	// Bruh moment when this would've worked perfectly first try if PocketMine-MP didn't use slightly different values
	const std::array<unsigned int, 28> serverColors{
			0,		  // no potion id 0
			0x7cafc6, // speed
			0x5a6c81,
			0xd9c043,
			0x4a4217,
			0x932423,
			0xf82423,
			0x430a09,
			0x22ff4c,
			0x551d4a,
			0xcd5cab,
			0x99453a,
			0xe49a3a,
			0x2e5299,
			0x7f8392,
			0x1f1f23, // blindness is bfc0c0 on nukkit
			0x1f1fa1, // todo nukkit night vision color 0x8b
			0x587653,
			0x484d48,
			0x4e9331,
			0x352a27,
			0xf87d23,
			0x2552a5,
			0xff00ff, // nukkit's saturation color, on pmmp it's the same as instant health (0xf82423) so w.e.
			0xceffff, // levitation
			0x4e9331,
			0x1dc2d1
			// todo: slow falling (has same val as lev plus they're pretty similar in effect so idrc)
	};


	const auto mix = [](std::vector<unsigned int> colors) -> unsigned int {
		std::vector<unsigned int>::size_type count = colors.size();
		unsigned int a = 0, r = 0, g = 0, b = 0;
		for (int i = 0; i < count; ++i) {
			unsigned int color = colors[i];
			a += color >> 24u & 0xFFu;
			r += color >> 16u & 0xFFu;
			g += color >> 8u & 0xFFu;
			b += color & 0xFFu;
		}
		a /= count;
		r /= count;
		g /= count;
		b /= count;
		return (a << 24u) | (r << 16u) | (g << 8u) | b;
	};

	// Obviously this cache takes longer to generate based on how many effects it can detect at the same time
	// Cache size: possible effects ^ max effects recognizable at once
	{
		std::vector<int> effects;
		std::vector<unsigned int> colors;
		auto impl = [&](int depth,
						auto &impl_ref) mutable -> void {
			for (int i = 0; i < serverColors.size(); i++) {
				if (!serverColors[i] || std::find(effects.begin(), effects.end(), i) != effects.end())
					continue;
				colors.resize(depth + 1);
				effects.resize(depth + 1);
				colors[depth] = serverColors[i];
				effects[depth] = i;
				potionCache.emplace(mix(colors), effects);
				if (depth < 3) // Simultaneous effects (+1)
					impl_ref(depth + 1, impl_ref);
			}
		};
		impl(0, impl);
	}

	const auto drawString = [](ExtRenderEvent &re, float fontSize, ImVec2 &pos, unsigned int color, std::string string) {
		re.drawList->AddText(re.font, fontSize, pos, color, string.c_str());
		pos.x += re.font->CalcTextSizeA(fontSize, FLT_MAX, -1.f, string.c_str(), NULL, NULL).x;
	};

	addDynamicListener<ExtRenderEvent>([&](auto &re) {
		if (!Util::client->isPlaying())
			return;

		bool details = false;

		Util::forEachPlayerSortedIf([&] (Player &player) {
			auto bb = player.aabb;
			bb.max.y += 0.2f;
			bb.offset(player.pos.subtract(player.prevPos).multiply(Util::mc->getTimer()->partialTicks - 1));

			unsigned int color = 0x7Fu * (player.getHurtTime() / 5);
			unsigned int outline = color | 0x80u << 24u;
			color |= 0xFFu << 24u;

			auto proj = Util::project(bb, re.width, re.height);

			if (!proj) {
				if (*tracers) {
					/*float ogPitch = Util::player->rotation.pitch;
					auto oldCam = *Util::client->getLevelRenderer()->projMat;
					Util::player->rotation.pitch = 90;
					Util::client->getLevelRenderer()->getLevelRendererPlayer().tickLevelRendererCamera();
					Util::player->rotation.pitch = ogPitch;
					*Util::client->getLevelRenderer()->projMat = oldCam;
					// todo fix offscreen calculation
					if (auto proj = Util::project(bb.min.add(player.width / 2, 0.2, 0), re.width, re.height, true))
						re.drawList->AddLine({re.width / 2, re.height / 2}, {proj->x , proj->y}, 0xFF000099);*/
				}

				return true;
			}

			float left = proj->min.x,
				bottom = proj->min.y,
				right = proj->max.x,
				top = proj->max.y;

			if (*tracers)
				re.drawList->AddLine({re.width / 2, re.height / 2}, {(left + right) / 2, bottom}, color);

			if (*box) {
				re.drawList->AddRect({left, top}, {right, bottom}, color);
				re.drawList->AddRect({left - 1, top - 1}, {right + 1, bottom + 1}, outline);
				re.drawList->AddRect({left + 1, top + 1}, {right - 1, bottom - 1}, outline);
			}

			if (*tags) {
				bool close = Util::player->distanceTo(player) < 3.5;

				auto name = player.getUnformattedNameTag() + ' ';
				name = name.substr(0, name.find('\n'));

				int healthVal = players[player.getUniqueID().data].health;
				auto healthColor = ColorFormat::ColorFromChar(healthVal > 16 ? 'a' : (healthVal > 12 ? '2' : (healthVal > 8 ? 'e' : (healthVal > 4 ? '6' : '4'))))->toABGR();

				auto healthStr = std::to_string(healthVal);
				auto distStr = std::to_string(round(Util::player->distanceTo(player))) + ' ';
				auto fullTag = (distStr + name + healthStr).c_str();

				auto size = re.font->CalcTextSizeA(close ? 24.f : 18.f, FLT_MAX, -1.f, fullTag, NULL, NULL);

				ImVec2 pos{(left + right - size.x) / 2, top - 12 - size.y};

				// Background
				re.drawList->AddRectFilled({pos.x - 3, pos.y - 2}, {pos.x + size.x + 3, pos.y + size.y + 6}, outline);

				// Health bar
				if (*health)
					re.drawList->AddRectFilled({pos.x + 1.5f, pos.y + size.y + 1}, {pos.x + (size.x * (healthVal / 20.f)) - 1.5f, pos.y + size.y + 4}, healthColor);

				auto team = ColorFormat::ColorFromChar(Util::getColorFromString(player.getNameTag()));
				float h = 0, s = 1, v = 1;
				if (team)
					Util::bgrToHsv(team->toABGR(), h, s, v);
				// Limit how dark the color is
				auto color = Util::hsvToBgr((h / 360) * 255, s * 255, std::max(.6f, v) * 255) | 0xFF << 24;

				drawString(re, close ? 24.f : 18.f, pos, ColorFormat::ColorFromChar('b')->toABGR() | 0xFF << 24, distStr);
				drawString(re, close ? 24.f : 18.f, pos, color, name);
				if (*health)
					drawString(re, close ? 24.f : 18.f, pos, healthColor, healthStr);

				// TODO: Render items amd potion icons

				auto viewDel = Util::wrap(Util::getRotationsTo(player)).distance(Util::player->rotation);

				if (details || viewDel.yaw + viewDel.pitch > 90)
					return true;

				pos.y = bottom + 6;

				if (*armor)
					for (char armorSlot = 0; armorSlot < 5; ++armorSlot) {
						ItemStack &stack = player.getArmor((ArmorSlot) armorSlot);
						if (!stack.isNull()) {
							auto armorName = stack.getName() + ' ';
							int dura = stack.getDamageValue() > 0 ? (((stack.getMaxDamage() - stack.getDamageValue()) / stack.getMaxDamage()) * 100) : 0; // TODO: Percent, does it even send dura?
							auto duraStr = std::to_string(dura);

							size = re.font->CalcTextSizeA(ImGui::GetDrawListSharedData()->FontSize, FLT_MAX, -1.f, (armorName + (dura ? duraStr : "")).c_str(), NULL, NULL);
							pos.x = (left + right - size.x) / 2;

							re.drawList->AddRectFilled({pos.x - 3, pos.y - 2}, {pos.x + size.x + 3, pos.y + size.y + 6}, outline);

							// BGR?
							drawString(re, 18.f, pos, 0xC8FF00/*stack.getColor().toABGR()*/, armorName);
							if (dura)
								drawString(re, 18.f, pos, ColorFormat::ColorFromChar(dura >= 80 ? 'a' : (dura >= 60 ? '2' : (dura >= 40 ? 'e' : (dura >= 20 ? '6' : '4'))))->toABGR(), duraStr);

							pos.y += size.y + 10;

							details = true;
						}
					}

				if (*effects) {
					// Bleh. Static initialization order because the game isn't initialized when corruption is.
					const static auto effectNames = []() {
						std::array<std::string, 28> ret;
						for (int i = 1; i < 28; ++i) {
							char p = ' ';
							ret[i] = MobEffect::getById(i)->getResourceName();
							std::for_each(ret[i].begin(), ret[i].end(), [&](char &c) {
								if (p == ' ' && ::isalpha(c)) c = std::toupper(c); // Uppercase words
								if (c == '_') c = ' '; // Replace underscores with spaces
								p = c;
							});
						}
						return ret;
					}();

					if (details)
						pos.y += 3; // Space between armor and effects

					for (const auto &eff : players[player.getUniqueID().data].effects) {
						auto &effectName = effectNames[eff->getId()];

						size = re.font->CalcTextSizeA(ImGui::GetDrawListSharedData()->FontSize, FLT_MAX, -1.f, effectName.c_str(), NULL, NULL);
						pos.x = (left + right - size.x) / 2;

						re.drawList->AddRectFilled({pos.x - 3, pos.y - 2}, {pos.x + size.x + 3, pos.y + size.y + 6}, outline);

						drawString(re, 18.f, pos, eff->getColor().toABGR() | 0xFF << 24, effectName);

						pos.y += size.y + 10;

						details = true;
					}
				}
			}

			return true;
		}, [](auto player) {
			return Util::player->distanceTo(player) > 1 && Util::valid(player);
		}, DistanceComparator{});
	});


	addDynamicListener<PostReadEvent<SetActorDataPacket>>([&](auto &pe) {
		// TODO: Player data tracking in the core (motion too)
		// The server actually will actually send a lot of useful information about players that the client doesn't even bother to deserialize normally
		// Expected motion is one; which is useful for detecting anti knockback and such but the client doesn't even set the motion
		auto it = players.find(pe.packet.rid);
		if (it == players.end())
			return;
		auto &data = it->second;
		for (auto *item : pe.packet.entries) {
			if (item->id == 1) { // Health, sent on nukkit and vanilla iirc
				data.health = item->intVal;
			} else if (item->id == 8) { // Particle color
				auto color = item->intVal;
				if (color) {
					color &= 0xFFFFFF; // PMMP sends alpha while NukkitX and possible others don't
					auto result = potionCache.find(color);
					if (result != potionCache.end()) {
						auto &ids = result->second;
						data.effects.clear();
						std::transform(ids.begin(), ids.end(), std::back_inserter(data.effects), MobEffect::getById);
					}
				} else
					data.effects.clear();
			// Same with underwater/suffocation ticks which I can't seem to properly figure out... Probably casting to the wrong type
			} else if (item->id == 7) { // Air ticks
				// TODO: Not sending right?
				data.air = item->intVal;
			}
		}
	})->addDynamicListener<PostReadEvent<RemoveActorPacket>>([&](auto &pe) {
		players.erase(pe.packet.uid.data);
	})->addDynamicListener<PreReadEvent<AddPlayerPacket>>([&](auto &pe) {
		players.emplace(pe.packet.uid.data, PlayerData{20, {}});
	});
}
