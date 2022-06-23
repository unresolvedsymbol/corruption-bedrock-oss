cmake_minimum_required(VERSION 3.12)

project(corruption)

set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_FLAGS "-DANDROID -fdata-sections -ffunction-sections -funwind-tables -fstack-protector-strong -no-canonical-prefixes -mstackrealign -fno-addrsig -Wa,--noexecstack -g -Wformat -Werror=format-security -std=c++17 -fno-rtti ")
set(CMAKE_CXX_FLAGS "-DANDROID -fdata-sections -ffunction-sections -funwind-tables -no-canonical-prefixes -fno-addrsig -Wa,--noexecstack -g -Wformat -Wno-deprecated -Wno-deprecated-declarations ")
#set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=address")
set(CORRUPTION_LIBRARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/${ANDROID_ABI}")

include(bedrock-headers/CMakeLists.txt)

include_directories(glad)
include_directories(imgui)
include_directories(src)

add_library(
        corruption SHARED

        glad/glad.c

        imgui/imgui.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_widgets.cpp
        imgui/imgui_tables.cpp

        #src/events/PacketEventFactory.cpp
        src/Corruption.cpp
        src/Hooks.cpp
        src/util/Util.cpp
        src/util/Log.cpp
        src/util/NBSReader.cpp
        src/gui/imgui_impl.cpp
        src/module/Module.cpp
        src/module/ToggleModule.cpp

        src/module/impl/SimpleModules.cpp
        src/module/impl/Commands.cpp

        src/module/impl/combat/Aura.cpp
        src/module/impl/combat/Hitboxes.cpp
        src/module/impl/combat/AimBot.cpp
        src/module/impl/combat/AutoHeal.cpp
        src/module/impl/combat/BowAim.cpp

        src/module/impl/move/Glide.cpp
        src/module/impl/move/Speed.cpp
        src/module/impl/move/Velocity.cpp
        src/module/impl/move/Step.cpp

        src/module/impl/world/Nuker.cpp
        src/module/impl/world/Scaffold.cpp

        src/module/impl/render/ESP.cpp
        src/module/impl/render/HUD.cpp

        src/module/impl/Keybinds.cpp
        src/module/impl/move/NoFall.cpp
        src/module/impl/move/RideSpeed.cpp
        src/module/impl/AntiCrash.cpp
        src/module/impl/world/NoteBot.cpp
        src/module/impl/world/SongStealer.cpp
        src/module/impl/world/Wallhack.cpp
        src/module/impl/world/Phase.cpp
        src/module/impl/move/Sprint.cpp
        src/module/impl/FancyChat.cpp
        src/module/impl/AntiCheat.cpp
        src/util/PatchUtils.cpp)

target_link_libraries(corruption PRIVATE ${CORRUPTION_LIBRARY_DIR}/libc++_shared.so ${CORRUPTION_LIBRARY_DIR}/libminecraftpe.so ${CORRUPTION_LIBRARY_DIR}/Roboto_Regular)

#if (${ANDROID_ABI} STREQUAL "armeabi-v7a")
#    target_link_libraries(corruption PRIVATE ${CORRUPTION_LIBRARY_DIR}/libsubstrate.so)
#endif()

# TODO BlockLauncher APK subproject or something
