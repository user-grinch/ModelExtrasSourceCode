local PSDK_DIR = os.getenv("PLUGIN_SDK_DIR")

-- 🔍 Recursively add include directories
function addIncludeDirs(root)
    for _, dir in ipairs(os.matchdirs(root .. "/*")) do
        includedirs { dir }
        addIncludeDirs(dir)
    end
end

workspace "ModelExtras"
    location "../build"
    architecture "x86"
    platforms { "Win32" }
    language "C++"
    cppdialect "C++latest"
    characterset "MBCS"
    staticruntime "On"

    configurations {
        "Debug",
        "Release"
    }

project "ModelExtras"
    kind "SharedLib"
    targetextension ".asi"
    targetdir "../build/bin"

    -- 🧩 Source & PCH
    files { "../src/**" }
    pchheader "pch.h"
    pchsource "../src/pch.cpp"

    -- 🧠 Preprocessor Definitions
    defines {
        "_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING",
        "_CRT_SECURE_NO_WARNINGS",
        "_CRT_NON_CONFORMING_SWPRINTFS",
        "PLUGIN_SGV_10US",
        "MODELEXTRAS_DEV",
        "GTASA",
        "RW"
    }

    -- 📁 Include Directories
    addIncludeDirs("../src/")
    includedirs {
        "../include/",
        "../src/",
        PSDK_DIR .. "/*",
        PSDK_DIR .. "/plugin_sa/**",
        PSDK_DIR .. "/shared/**"
    }

    -- 📦 Library Directories
    libdirs {
        PSDK_DIR .. "/output/lib",
        "../lib/",
        "build/bin/"
    }

    -- 🔗 Linking
    links {
        "delayimp",
        "GrinchTrainerSA"
    }

    linkoptions {
        "/SAFESEH:NO",                          -- Allow unsafe exception handlers
        "/RTC1",                                -- Runtime checks (stack, uninit vars)
        "/EHsc",                                -- C++ exception model
        "/DELAYLOAD:GrinchTrainerSA.asi",       -- Delay-load plugin
        "/DEBUG",                               -- Generate debug info
        "/MAP"                                  -- Optional: generate map file
    }

    -- 🧱 Global Build Options (for all configs)
    filter "configurations:*"
        buildoptions {
            "/Od",     -- Disable optimization
            "/Ob0",    -- Disable inlining
            "/Oy-"     -- Disable frame pointer optimization
        }

    -- 🐞 Debug Config
    filter "configurations:Debug"
        symbols "On"
        optimize "Off"
        links { "plugin_d.lib" }

    -- 🚀 Release Config
    filter "configurations:Release"
        symbols "Off"
        optimize "Speed"
        links { "plugin.lib" }
