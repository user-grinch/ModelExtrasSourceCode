cmake_minimum_required(VERSION 4.1.2)
project(ModelExtras LANGUAGES CXX)

# ============================================================
# Environment checks
# ============================================================

if(NOT DEFINED ENV{PLUGIN_SDK_DIR})
    message(FATAL_ERROR "PLUGIN_SDK_DIR2 environment variable is not set")
endif()
set(PLUGIN_SDK_DIR $ENV{PLUGIN_SDK_DIR})

if(DEFINED ENV{GTA_SA_DIR})
    set(DEPLOY_DIR $ENV{GTA_SA_DIR})
else ()
    message(FATAL_ERROR "GTA_SA_DIR environment variable is not set")
endif()

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# ============================================================
# Enable C++23 + experimental MSVC modules
# ============================================================

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================================
# Collect source files
# ============================================================

# All C++ source files
file(GLOB_RECURSE CPP_SOURCES
        src/*.cpp
        include/imgui/*.cpp
        include/coreutils/imgui/fonts/*.cpp
        include/coreutils/imgui/rw/*.cpp
        include/coreutils/imgui/widgets/*.cpp
)

# All module files (.ixx and .cppm)
file(GLOB_RECURSE MODULE_FILES
        src/**/*.ixx
        src/**/*.cppm
)

# ============================================================
# Target
# ============================================================

add_library(ModelExtras SHARED
        src/view/tabs/inspector.cpp
        src/view/tabs/editor.cpp
        src/view/hooks.cpp
        src/view/tabs/status.cpp
        src/view/tabs/lights.cpp
        src/view/common.hpp
        src/view/tabs/modules/metadata.hpp
        src/view/tabs/modules/IEditor.hpp
        src/view/tabs/modules/light.hpp
        src/view/tabs/modules/sirens.hpp
        src/view/tabs/modules/spoiler.hpp
        src/view/tabs/modules/exhaust.hpp
        src/view/tabs/modules/door.hpp
        src/view/tabs/modules/roof.hpp
)

# Add modules
if(MODULE_FILES)
    target_sources(ModelExtras
            PRIVATE
            FILE_SET cxx_modules TYPE CXX_MODULES FILES
            ${MODULE_FILES}
    )
endif()

# Add normal C++ sources
target_sources(ModelExtras PRIVATE ${CPP_SOURCES})

# ============================================================
# Include directories
# ============================================================

target_include_directories(ModelExtras PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/features
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/coreutils
        ${CMAKE_CURRENT_SOURCE_DIR}/include/imgui
        ${CMAKE_CURRENT_SOURCE_DIR}/include/nlohmann
        ${CMAKE_CURRENT_SOURCE_DIR}/include/spdlog

        ${PLUGIN_SDK_DIR}
        ${PLUGIN_SDK_DIR}/plugin_sa
        ${PLUGIN_SDK_DIR}/plugin_sa/game_sa
        ${PLUGIN_SDK_DIR}/plugin_sa/game_sa/meta
        ${PLUGIN_SDK_DIR}/plugin_sa/game_sa/rw
        ${PLUGIN_SDK_DIR}/shared
        ${PLUGIN_SDK_DIR}/shared/bass
        ${PLUGIN_SDK_DIR}/shared/comp
        ${PLUGIN_SDK_DIR}/shared/dxsdk
        ${PLUGIN_SDK_DIR}/shared/game
        ${PLUGIN_SDK_DIR}/shared/extender
        ${PLUGIN_SDK_DIR}/shared/extensions
)

# ============================================================
# Compile definitions
# ============================================================

target_compile_definitions(ModelExtras PRIVATE
        IMGUI_DEFINE_MATH_OPERATORS
        _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
        _CRT_SECURE_NO_WARNINGS
        _CRT_NON_CONFORMING_SWPRINTFS
        PLUGIN_SGV_10US
        MODELEXTRAS_DEV
        GTASA
        RW
)

# ============================================================
# Compiler options
# ============================================================

target_compile_options(ModelExtras PRIVATE
        $<$<CONFIG:Debug>:/std:c++latest /W3 /Od /Ob0 /Oy- /EHsc>
        $<$<CONFIG:Release>:/std:c++latest  /W3 /O2 /Ob2 /Ot /Oy- /EHsc>
)

# ============================================================
# Linking
# ============================================================

target_link_directories(ModelExtras PRIVATE
        ${PLUGIN_SDK_DIR}/output/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

target_link_libraries(ModelExtras PRIVATE
        delayimp.lib
        $<$<CONFIG:Debug>:plugin_d.lib>
        $<$<CONFIG:Release>:plugin.lib>
)

target_link_options(ModelExtras PRIVATE
        /SAFESEH:NO
        $<$<CONFIG:Debug>:/DEBUG /MAP /RTC1>
        $<$<CONFIG:Release>:/DEBUG /MAP /OPT:REF /OPT:ICF>
)

# ============================================================
# Output directories
# ============================================================

set_target_properties(ModelExtras PROPERTIES
        OUTPUT_NAME "ModelExtras"
        PREFIX ""
        SUFFIX ".asi"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_CURRENT_SOURCE_DIR}/build/bin/Debug
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/build/bin/Release
        ARCHIVE_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_CURRENT_SOURCE_DIR}/build/Debug
        ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/build/Release
)

# ============================================================
# Precompiled headers
# ============================================================

target_precompile_headers(ModelExtras PRIVATE src/pch.h)

# ============================================================
# Post-build deployment (ASI + PDB)
# ============================================================

if(DEFINED DEPLOY_DIR)
    file(MAKE_DIRECTORY ${DEPLOY_DIR})

    add_custom_command(TARGET ModelExtras POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:ModelExtras>
            ${DEPLOY_DIR}
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_PDB_FILE:ModelExtras>
            ${DEPLOY_DIR}
            COMMAND_EXPAND_LISTS
    )
endif()
