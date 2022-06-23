#include "util/StaticHook.h"
#include "util/Util.h"
#include <atomic>
#include <client/input/Keyboard.h>
#include "imgui.h"
#include "imgui_internal.h"
#include <gui/imgui_impl.h>
#include "item/EnchantUtils.h"
#include "util/Brightness.h"
#include "client/gui/screens/models/ClientInstanceScreenModel.h"
#include "events/impl/MiscEvents.h"
#include "events/EventChannel.h"
#include "glad.h"
#include "world/VanillaBlocks.h"
#include "network/protocol/UpdateBlockPacket.h"
#include "gui/Roboto_Regular.h"
#include "util/PatchUtils.h"
#include "events/PacketEventFactory.h"
#include "events/impl/RenderEvents.h"
#include "events/impl/BlockEvents.h"
#include "events/impl/PlayerEvents.h"
#include "util/serialize.h"
#include "client/input/Mouse.h"

#define SIZEHOOKS 64 // Allocate space to hold hooks for future destruction

// Poison alt checker dbs and ban evade lol
/*TClasslessInstanceHook(unsigned long long, _ZNK18UserAuthentication17getClientRandomIdEv) {
	return Util::generateRandomId(1).least;
}

TClasslessInstanceHook(std::string, _ZNK11AppPlatform11getDeviceIdEv) {
	auto did = Util::generateRandomId(0).asString();
	did.erase(std::remove(did.begin(), did.end(), '-'), did.end()); // Remove hyphens
	return did;
}*/

TInstanceHook(void, _ZN9Minecraft4initEv, Minecraft) {
	Log(1) << "Got Minecraft instance " << std::hex << this;
	original(Util::mc = this);
}

TInstanceHook(void, _ZN25ClientInstanceScreenModel15sendChatMessageERKNSt6__ndk112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE, ClientInstanceScreenModel, std::string const& message) {
	if (!EventChannel::fire<ChatEvent>({false, *this, const_cast<std::string &>(message)}).cancelled)
		original(this, message);
}

/*TClasslessInstanceHook(int, _ZNK11AppPlatform16getBuildPlatformEv) {
	return EventChannel::fire(OSEvent{1}).type;
}
TClasslessInstanceHook(int, _ZNK19AppPlatform_android16getBuildPlatformEv) {
	return EventChannel::fire(OSEvent{1}).type;
}
TClasslessInstanceHook(int, _ZNK19AppPlatform_android15getPlatformTypeEv) {
	return EventChannel::fire(OSEvent{1}).type;
}

TClasslessInstanceHook(int, _ZNK11AppPlatform15getPlatformTypeEv) {
	return EventChannel::fire(OSEvent{1}).type;
}
TClasslessInstanceHook(int, _ZNK11AppPlatform12getOSVersionEv) {
	return EventChannel::fire(OSEvent{1}).type;
}*/


/*TStaticHook(int, ZNK20MinecraftScreenModel16getBuildPlatformEv, MinecraftScreenModel) {
	return EventChannel::fire(OSEvent{1}).type;
}*/

// Game exit
/*TInstanceHook(void, _ZN14ClientInstance4quitERKSsS1_, ClientInstance, std::string const& param1, std::string const& param2) {
	delete Corruption::get();
	original(this, param1, param2);
}

TClasslessInstanceHook(void, _ZN16MapItemSavedData18replaceDecorationsESt6vectorISt10shared_ptrI13MapDecorationESaIS3_EES0_IN19MapItemTrackedActor8UniqueIdESaIS7_EE,
	std::vector<std::shared_ptr<MapDecoration>> decorations, std::vector<ActorUniqueID> ids) {
	EventChannel::fire(MapUpdateEvent{decorations, ids});
	return original(this, decorations, ids);
}*/

// Render/GUI hooks

// TODO: Is LevelRenderer::computeCameraPos(float) equivalent to setupCameraTransform?

// No water push
//TClasslessInstanceHook(void, _ZNK11LiquidBlock18handleEntityInsideER11BlockSourceRK8BlockPosP5ActorR4Vec3, BlockSource *, const BlockPos *, Vec3 &) {
//}

// TODO: Fix broken block/slowdown hooks

#if 0
TClasslessInstanceHook(void, _ZNK8WebBlock12entityInsideER11BlockSourceRK8BlockPosR5Actor, BlockSource &region, const BlockPos &pos, Actor &actor) {
	if (EventChannel::fire<SlowdownEvent>({}))
		original(this, region, pos, actor);
}

TClasslessInstanceHook(void, _ZNK19SweetBerryBushBlock12entityInsideER11BlockSourceRK8BlockPosR5Actor, BlockSource &region, const BlockPos &pos, Actor &actor) {
	if (EventChannel::fire<SlowdownEvent>({}))
		original(this, region, pos, actor);
}
#endif

// hmm _ZNK13SoulSandBlock18calcGroundFrictionER3MobRK8BlockPos ?
/*TClasslessInstanceHook(void, _ZNK13SoulSandBlock12entityInsideER11BlockSourceRK8BlockPosR5Actor, BlockSource &region, const BlockPos &pos, Actor &actor) {
	if (EventChannel::fire<SlowdownEvent>({}))
		original(this, region, pos, actor);
}*/

TClasslessInstanceHook(Brightness, _ZNK11BlockLegacy16getLightEmissionERK5Block, const Block &block) {
	return EventChannel::fire(BlockLightEvent{block, original(this, block)}).value;
}

TClasslessInstanceHook(bool, _ZN13BlockGraphics15isFullAndOpaqueERK5Block, const Block &block) {
	return !EventChannel::fire(BlockTranslucentEvent{block, !original(this, block)}).opaque;
}

// I forgot what property 0x20 is lol I saw it in IDA it makes it glass like or something where the transparent color renders...
TInstanceHook(bool, _ZNK5Block11hasPropertyE13BlockProperty, Block, unsigned char property) {
	return property == 0x20 ? !EventChannel::fire(BlockTranslucentEvent{*this, !original(this, property)}).opaque : original(this, property);
}

TClasslessInstanceHook(unsigned int, _ZNK11BlockLegacy8getColorER11BlockSourceRK8BlockPosRK5Block, BlockSource &region, const BlockPos &pos, const Block &block) {
	return EventChannel::fire(BlockColorEvent{block, original(this, region, pos, block)}).color;
}

TClasslessInstanceHook(int, _ZNK11BlockLegacy14getRenderLayerERK5BlockR11BlockSourceRK8BlockPos, const Block &block, BlockSource &region, const BlockPos &pos) {
	return EventChannel::fire(BlockRenderLayerEvent{block, block.getRenderLayer()}).value;
}

// GUI

using AnyFunc = void *(*)();
using ProcAddrFunc = AnyFunc (*)(const char *);
using GlGetStringFunc = const char *(*)(int);

extern "C" AnyFunc eglGetProcAddress(const char *) { return nullptr; };

//TClasslessInstanceHook(void, _ZN19LevelRendererCamera6renderER22BaseActorRenderContextRK16ViewRenderObjectR15IClientInstanceR28LevelRendererCommandListInit, BaseActorRenderContext &ctx, const void* vro, ClientInstance &client, const void* commands) {
//	original(this, ctx, vro, client, commands);

TInstanceHook(void, _ZN18ClientInputHandler6renderER13ScreenContext, ClientInputHandler, ScreenContext &ctx) {
	original(this, ctx);

	// TODO
	//ctx.getFont().drawShadow(ctx.getScreenContext(), "Hello from Minecraft's internal rendering!", 50.f, 50.f, Color::RED, true, nullptr, 3.f);
	//if (Util::client->getClientSceneStack()->getSize() == 2)
	//ctx.getItemRenderer().renderGuiItemNew(ctx, Util::player->getSelectedItem(), 1, 50.f, 50.f, 50.f, 50.f, 50.f, false);

	static bool init = false, fail = false;

	if (fail)
		return;

	static ImFont *roboto;
	//static ImDrawList *externalDrawList;
	static ImGuiContext *imGuiContext;
	//EventChannel::fire<RenderCameraEvent>({ctx});
	if (!init) {
		//if (!gladLoadGLES2Loader(reinterpret_cast<GLADloadproc>(dlsym(dlopen("libEGL.so", RTLD_LAZY), "eglGetProcAddress")))) {
		if (!gladLoadGLES2Loader(reinterpret_cast<GLADloadproc>(eglGetProcAddress))) {
			std::cerr << "Couldn't init GLAD." << std::endl;
			fail = true;
			return;
		}

		IMGUI_CHECKVERSION();
		imGuiContext = ImGui::CreateContext();
		ImGuiIO &io = ImGui::GetIO();
		(void) io;

		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.KeyMap[ImGuiKey_Space] = 0;
		io.KeyMap[ImGuiKey_LeftArrow] = 1;
		io.KeyMap[ImGuiKey_UpArrow] = 2;
		io.KeyMap[ImGuiKey_RightArrow] = 3;
		io.KeyMap[ImGuiKey_DownArrow] = 4;
		io.KeyMap[ImGuiKey_Escape] = 5;

		ImGui_ImplOpenGL3_Init();

		ImGuiStyle &style = ImGui::GetStyle();
		style.FramePadding = ImVec2(4.0f, 2.0f);
		style.ItemSpacing = ImVec2(8.0f, 2.0f);
		style.WindowRounding = 0;
		style.ChildRounding = 0;
		style.FrameRounding = 0;
		style.WindowBorderSize = 0;
		style.FrameBorderSize = 1;
		style.PopupBorderSize = 0;
		style.PopupRounding = 0;
		style.ScrollbarRounding = 0;
		style.TabRounding = 0;
		style.GrabMinSize = 8;
		style.GrabRounding = 0;

		ImVec4 *colors = style.Colors;
		colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
		colors[ImGuiCol_ChildBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
		colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.21f, 0.22f, 0.54f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.40f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.18f, 0.18f, 0.18f, 0.67f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.29f, 0.29f, 0.29f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(0.44f, 0.44f, 0.44f, 0.40f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.46f, 0.47f, 0.48f, 1.00f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.42f, 0.42f, 0.42f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.70f, 0.70f, 0.70f, 0.31f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.70f, 0.70f, 0.70f, 0.80f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.48f, 0.50f, 0.52f, 1.00f);
		colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.72f, 0.72f, 0.72f, 0.78f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.91f, 0.91f, 0.91f, 0.25f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.81f, 0.81f, 0.81f, 0.67f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.46f, 0.46f, 0.46f, 0.95f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.73f, 0.60f, 0.15f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.87f, 0.87f, 0.87f, 0.35f);

		//colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

		colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);

		roboto = io.Fonts->AddFontFromMemoryTTF(const_cast<uint8_t*>(Roboto_Regular_ttf), Roboto_Regular_ttf_size, 18.f);

		init = true;

		return;
	}

	ImGui::SetCurrentContext(imGuiContext);

	//float mX = Mouse::getX(), mY = Mouse::getY();

	ImGuiIO &io = ImGui::GetIO();

	float viewport[4];
	glGetFloatv(GL_VIEWPORT, (GLfloat *) viewport);
	io.DisplaySize.x = viewport[2];
	io.DisplaySize.y = viewport[3];

	/*if (mX == 0 && mY == 0)
		io.MousePos = {-FLT_MAX, -FLT_MAX};
	else
		io.MousePos = {mX, mY};*/

	io.MouseDown[0] = Mouse::isButtonDown(1);
	io.MouseDown[1] = Mouse::isButtonDown(2);

	io.KeysDown[0] = Keyboard::_states[34]; // PgDn = Insert
	io.KeysDown[1] = Keyboard::_states[37]; // Left
	io.KeysDown[2] = Keyboard::_states[38]; // Up
	io.KeysDown[3] = Keyboard::_states[39]; // Right
	io.KeysDown[4] = Keyboard::_states[40]; // Down
	io.KeysDown[5] = Keyboard::_states[33]; // PgUp = Cancel GUESS

	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();
	ImGui::PushFont(roboto);

	ImGui::PopFont();
	ImGui::EndFrame();
	ImGui::Render();
	//ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	// Begin custom rendering

	ImDrawList externalDrawList{ImGui::GetDrawListSharedData()};

	externalDrawList.PushClipRectFullScreen();
	externalDrawList.PushTextureID(roboto->ContainerAtlas->TexID);

	EventChannel::fire<ExtRenderEvent>(
			{roboto, &externalDrawList, static_cast<float>(viewport[2]), static_cast<float>(viewport[3])});

	auto externalDrawData = new ImDrawData;
	externalDrawData->Valid = true;
	ImDrawList *list = {&externalDrawList};
	externalDrawData->CmdLists = &list;
	externalDrawData->CmdListsCount = 1;
	externalDrawData->TotalVtxCount = list->VtxBuffer.Size;
	externalDrawData->TotalIdxCount = list->IdxBuffer.Size;
	externalDrawData->DisplayPos = ImVec2(0.0f, 0.0f);
	externalDrawData->DisplaySize = io.DisplaySize;
	externalDrawData->FramebufferScale = io.DisplayFramebufferScale;
	ImGui_ImplOpenGL3_RenderDrawData(externalDrawData);
}

// Actor/Entity hooks

/*TInstanceHook(bool, _ZNK5Actor9isInWaterEv, Actor) {
	return reinterpret_cast<Actor*>(this) == Util::player ? EventChannel::fire(InWaterEvent{original(this)}).value : original(this);
}*/

// Player hooks

//TInstanceHook(void, _ZN14ClientInstance20onClientCreatedLevelENSt6__ndk110unique_ptrI5LevelNS0_14default_deleteIS2_EEEENS1_I11LocalPlayerNS3_IS6_EEEE, ClientInstance, Level *level, LocalPlayer *localPlayer) {
void *clientCreatedLevelPtr;
void *clientCreatedLevelHook(ClientInstance *_this, Level **level, LocalPlayer **player) {
	Util::player = *player;
	std::cerr << (*player)->getNameTag() << std::endl;
	Util::client = _this;
	Util::game = _this->getMinecraftGame();
	Util::input = _this->getMoveTurnInput(); //&(*player)->getMoveInputHandler();
	return reinterpret_cast<decltype(&clientCreatedLevelHook)>(clientCreatedLevelPtr)(_this, level, player);
}

TInstanceHook(float, _ZN11LocalPlayer22getFieldOfViewModifierEv, LocalPlayer) {
	return EventChannel::fire<FOVEvent>({original(this)}).modifier;
}

void *moveInputTickPtr;
void moveInputTickHook(MoveInputHandler *inputHandler, void **playerMovementProxy) {
	reinterpret_cast<decltype(&moveInputTickHook)>(moveInputTickPtr)(inputHandler, playerMovementProxy);
	EventChannel::fire<MoveInputEvent>({});
}

/*
 * TODO: Cleaner item hijacking...
 */
// Exploit removed

// AntiCrash measure, null motives. Don't know if this was fixed, probably was but best safe than sorry when fucking around with crash exploits
TClasslessInstanceHook(void, _ZN16PaintingRenderer8_getMeshER22BaseActorRenderContextPK6Motive, BaseActorRenderContext &context, const void *motive) {
	if (motive)
		original(this, context, motive);
}

// Block Actor Chams
/*TClasslessInstanceHook(void, _ZN26BlockActorRenderDispatcher6renderER22BaseActorRenderContextR11BlockSourceR10BlockActorRK5BlockbRKN3mce11MaterialPtrEPKNS9_13ClientTextureEiN6nonstd13optional_lite8optionalIN6dragon14RenderMetadataEEE,
	BaseActorRenderContext &context, BlockSource &region, const Actor &actor, const Block &block, const Vec3 &pos, bool unk, const mce::MaterialPtr &material, mce::MaterialPtr &texturePtr, int i) {
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0f, -1100000.0f);
	original(this, context, region, actor, block, pos, unk, material, texturePtr, i);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0f, 1100000.0f);
}*/

#if 0
TClasslessInstanceHook(void, _ZN21ActorRenderDispatcher6renderER22BaseActorRenderContextR5Actorb, BaseActorRenderContext &context, Actor &actor, bool ignoreLighting) {
	bool chams = false;
	/*if (auto corruption = Corruption::get()) { // Won't be needed after unhooking is done
		auto esp = corruption->modules().get<ESP>();
		chams = esp.get() && *esp.chams;
	}*/
	if (chams) {
		glEnable(GL_POLYGON_OFFSET_FILL);
		//glPolygonOffset(1.0f, -1100000.0f);
		glPolygonOffset(1.0f, -1100000.0f);
	}
	// TODO: Write an ES 3.0 outline shader and attach it around this
	// Project the entity bounding box and test near pixels in that area
	original(this, context, actor, true);
	if (chams) {
		glPolygonOffset(1.0f, 1100000.0f);
		glDisable(GL_POLYGON_OFFSET_FILL);
	}
}
#endif

void *originalNormalTickPtr;
void localPlayerNormalTickHook(LocalPlayer *localPlayer) {
	/*if (!Util::player || Util::player != localPlayer) {
		Util::player = localPlayer;
		Log(1) << "Got LocalPlayer at " << std::hex << localPlayer;
		Util::client = &(localPlayer->getClientInstance());
		Log(1) << "Got ClientInstance at " << std::hex << Util::client;
		Util::game = Util::client->getMinecraftGame();
		Log(1) << "Got MinecraftGame at " << std::hex << Util::game;
		Util::input = Util::client->getMoveTurnInput();
		Log(1) << "Got MoveInputHandler at " << std::hex << Util::input;
	} else*/
	EventChannel::fire<PlayerTick>({});
	reinterpret_cast<decltype(&localPlayerNormalTickHook)>(originalNormalTickPtr)(localPlayer);
}

void *originalRideTickPtr;
void playerRideTickHook(Player *player) {
	EventChannel::fire<RideTick>({player->getRide()});
	reinterpret_cast<decltype(&playerRideTickHook)>(originalRideTickPtr)(player);
}

void *originalMovePtr;
bool localPlayerMoveHook(LocalPlayer *localPlayer, Vec3 const &motion) {
	EventChannel::fire<MoveEvent>({const_cast<Vec3 &>(motion)});
	return reinterpret_cast<decltype(&localPlayerMoveHook)>(originalMovePtr)(localPlayer, motion);
}

TVHook(void *, MinecraftHandle(), _LocalPlayer, _ZN_lmao, Vec3 const &motion)

// Packet hooks

void *normalSendPtr;
void loopbackSendHook(PacketSender *sender, Packet &packet) {
	if (!EventChannel::fire<PreSendEvent<Packet>>(packet))
		return;

	if (!PacketEventFactory::fireEvent(PacketEventFactory::Type::PreSend, packet))
		return;

	reinterpret_cast<decltype(&loopbackSendHook)>(normalSendPtr)(sender, packet);
	PacketEventFactory::fireEvent(PacketEventFactory::Type::PostSend, packet);

	EventChannel::fire<PostSendEvent<Packet>>(packet);
}

/*TStaticHook(Brightness, _ZNK11BlockLegacy16getLightEmissionERK5Block, BlockLegacy, const Block &block) {
	return EventChannel::fire(BlockLightEvent{block, BlockLegacy::getLightEmission(block)}).value;
}

TStaticHook(unsigned int, _ZNK11BlockLegacy8getColorER11BlockSourceRK8BlockPosRK5Block, BlockLegacy, BlockSource &region, const BlockPos &pos, const Block &block) {
	return EventChannel::fire(BlockColorEvent{block, BlockLegacy::getColor(region, pos, block)}).color;
}

TStaticHook(int, _ZNK11BlockLegacy14getRenderLayerERK5BlockR11BlockSourceRK8BlockPos, BlockLegacy, const Block &block, BlockSource &region, const BlockPos &pos) {
	return EventChannel::fire(BlockRenderLayerEvent{block, block.getRenderLayer()}).value;
}*/

void *originalBlockLightPtr;
Brightness blockLightHook(BlockLegacy *pThis, const Block &block) {
	return EventChannel::fire(BlockLightEvent{block, reinterpret_cast<decltype(&blockLightHook)>(originalBlockLightPtr)(pThis, block)}).value;
}

void *originalBlockColorPtr;
unsigned int blockColorHook(BlockLegacy *pThis, BlockSource &region, const BlockPos &pos, const Block &block) {
	return EventChannel::fire(BlockColorEvent{block, reinterpret_cast<decltype(&blockColorHook)>(originalBlockColorPtr)(pThis, region, pos, block)}).color;
}

void *originalRenderLayerPtr;
int blockRenderLayerHook(BlockLegacy *pThis, const Block &block, BlockSource &region, const BlockPos &pos) {
	return EventChannel::fire(BlockRenderLayerEvent{block, reinterpret_cast<decltype(&blockRenderLayerHook)>(originalRenderLayerPtr)(pThis, block, region, pos)}).value;
}

void *originalStuckInBlockPtr;
void stuckInBlockHook(LocalPlayer *localPlayer, float value) {
	if (EventChannel::fire(SlowdownEvent{}))
		reinterpret_cast<decltype(&stuckInBlockHook)>(originalStuckInBlockPtr)(localPlayer, value);
}

// Find some packet creation hook, get PacketHandlerDispatcherInstance templated vtable
// Replace index 2
// _ZNK31PacketHandlerDispatcherInstanceI16MovePlayerPacketLb0EE6handleERK17NetworkIdentifierR16NetEventCallbackRNSt6__ndk110shared_ptrI6PacketEE
inline std::array<uintptr_t, 164> originalPacketHandle{};
void packetHandleHook(void *_this, NetworkIdentifier const &identifier, NetEventCallback &cbk, Packet **packet) {
	auto original = reinterpret_cast<decltype(&packetHandleHook)>(originalPacketHandle[(*packet)->getId()]);

	// Looks strange but cancellable events can implicitly converted to bool that is !cancelled
	if (!(EventChannel::fire<PreReadEvent<Packet>>(**packet) &&
		PacketEventFactory::fireEvent(PacketEventFactory::Type::PreRead, **packet)))
		return;

	original(_this, identifier, cbk, packet);

	PacketEventFactory::fireEvent(PacketEventFactory::Type::PostRead, **packet);
	EventChannel::fire<PostReadEvent<Packet>>(**packet);
}

TClasslessInstanceHook(Packet **, _ZN16MinecraftPackets12createPacketE18MinecraftPacketIds, int packetId) {
	Packet **packet = original(this, packetId);
	if (!*packet || originalPacketHandle[packetId])
		return packet;
	uintptr_t *handlerVt = (*packet)->handlerThunk[0]; // DispatcherInstance vtable
	// 2 is index of handle function
	if (handlerVt[2] == reinterpret_cast<uintptr_t>(&packetHandleHook))
		return packet;
	originalPacketHandle[packetId] = handlerVt[2];
	handlerVt[2] = reinterpret_cast<uintptr_t>(&packetHandleHook);
	return packet;
};

static int hooksInit = []() -> int {
	Log(1) << "Applying hooks...";
	reinterpret_cast<void * &>(Keyboard::_states) = dlsym(MinecraftHandle(), "_ZN8Keyboard7_statesE");
	// syms are "vtable for x"
	void *clientInstanceSym = dlsym(MinecraftHandle(), "_ZTV14ClientInstance");
	if (clientInstanceSym) {
		Log(1) << "Hijacking ClientInstance vtable";
		void **clientInstanceVt = &(reinterpret_cast<void **>(clientInstanceSym))[2];
		PatchUtils::VtableReplaceHelper clientInstanceVtr(MinecraftHandle(), clientInstanceVt, clientInstanceVt);
		clientInstanceVtr.replaceOrig("_ZN14ClientInstance20onClientCreatedLevelENSt6__ndk110unique_ptrI5LevelNS0_14default_deleteIS2_EEEENS1_I11LocalPlayerNS3_IS6_EEEE", &clientCreatedLevelHook, &clientCreatedLevelPtr);
	}
	void *localPlayerSym = dlsym(MinecraftHandle(), "_ZTV11LocalPlayer");
	if (localPlayerSym) {
		Log(1) << "Hijacking LocalPlayer vtable";
		void **localPlayerVt = &(reinterpret_cast<void **>(localPlayerSym))[2];
		//PatchUtils::dumpVtable(localPlayerVt);
		PatchUtils::VtableReplaceHelper localPlayerVtr(MinecraftHandle(), localPlayerVt, localPlayerVt);
		localPlayerVtr.replaceOrig("_ZN11LocalPlayer10normalTickEv", &localPlayerNormalTickHook, &originalNormalTickPtr);
		localPlayerVtr.replaceOrig("_ZN11LocalPlayer4moveERK4Vec3", &localPlayerMoveHook, &originalMovePtr);
		localPlayerVtr.replaceOrig("_ZN6Player8rideTickEv", &playerRideTickHook, &originalRideTickPtr);
		localPlayerVtr.replaceOrig("_ZN5Actor16makeStuckInBlockERK4Vec3", &stuckInBlockHook, &originalStuckInBlockPtr);
	}
	void *loopbackPacketSenderSym = dlsym(MinecraftHandle(), "_ZTV20LoopbackPacketSender");
	if (loopbackPacketSenderSym) {
		Log(1) << "Hijacking LoopbackPacketSender vtable";
		void **loopbackPacketSenderVt = &(reinterpret_cast<void **>(loopbackPacketSenderSym))[2];
		PatchUtils::VtableReplaceHelper loopbackPacketSenderVtr(MinecraftHandle(), loopbackPacketSenderVt, loopbackPacketSenderVt);
		loopbackPacketSenderVtr.replaceOrig("_ZN20LoopbackPacketSender4sendER6Packet", &loopbackSendHook, &normalSendPtr);
	}
	void *blockLegacySym = dlsym(MinecraftHandle(), "_ZTV11BlockLegacy");
	if (blockLegacySym) {
		Log(1) << "Hijacking BlockLegacy vtable";
		void **blockLegacyVt = &(reinterpret_cast<void **>(blockLegacySym))[2];
		PatchUtils::VtableReplaceHelper blockLegacyVtr(MinecraftHandle(), blockLegacyVt, blockLegacyVt);
		blockLegacyVtr.replaceOrig("_ZNK11BlockLegacy16getLightEmissionERK5Block", &blockLightHook, &originalBlockLightPtr);
		blockLegacyVtr.replaceOrig("_ZNK11BlockLegacy8getColorER11BlockSourceRK8BlockPosRK5Block", &blockColorHook, &originalBlockColorPtr);
		blockLegacyVtr.replaceOrig("_ZNK11BlockLegacy14getRenderLayerERK5BlockR11BlockSourceRK8BlockPos", &blockRenderLayerHook, &originalRenderLayerPtr);
	}
	void *clientMoveInputHandlerSym = dlsym(MinecraftHandle(), "_ZTV22ClientMoveInputHandler");
	if (clientMoveInputHandlerSym) {
		Log(1) << "Hijacking ClientMoveInputHandler vtable";
		void **clientMoveInputHandlerVt = &(reinterpret_cast<void **>(clientMoveInputHandlerSym))[2];
		PatchUtils::VtableReplaceHelper clientMoveInputHandlerVtr(MinecraftHandle(), clientMoveInputHandlerVt, clientMoveInputHandlerVt);
		clientMoveInputHandlerVtr.replaceOrig("_ZN22ClientMoveInputHandler4tickER20IPlayerMovementProxy", &moveInputTickHook, &moveInputTickPtr);
	}
	mcpelauncher_hook2_apply();
	// TODO: Fix ng modloader orig base offsetting
	slideAddress = realOffset - *scuffedOffset;
	Log(1) << "Real: " << std::hex << realOffset << " scuffed: " << *scuffedOffset << " slide: " << std::hex << slideAddress;
	HookManager::HookInstance *hookPtr;
	for (int i = 0; (hookPtr = reinterpret_cast<decltype(hookPtr)>(hookCache[i])); ++i)
		*hookPtr->orig += slideAddress;
	Log(1) << "End init";
	return 0;
}();
