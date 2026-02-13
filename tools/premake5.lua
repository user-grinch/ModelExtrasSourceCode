PSDK_DIR = os.getenv("PLUGIN_SDK_DIR")
GAME_DIR = "/mnt/p2/Games/GTA San Andreas/"

function addIncludeDirs(root)
    for _, dir in ipairs(os.matchdirs(root .. "/*")) do
        includedirs { dir }
        addIncludeDirs(dir)
    end
end

workspace "ModelExtras"
    configurations { "Debug", "Release" }
    architecture "x86"
    system "Windows"
    platforms { "Win32" } 
    language "C++"
    cppdialect "C++23"
    staticruntime "On"
    location "../build"
    targetdir "../build/bin"

project "ModelExtras"
    kind "SharedLib"
    targetextension ".asi"

    pchheader "pch.h"
    pchsource "../src/pch.cpp"

    defines { 
        "PLUGIN_SGV_10US",
        "MODELEXTRAS_DEV",
        "GTASA",
        "RW",
    }

    includedirs {
        "../include/",
        "../include/imgui/",
        "../include/coreutils/",
        "../src/",
        "../src/features/",
        PSDK_DIR .. "/*",
        PSDK_DIR .. "/plugin_sa/*",
        PSDK_DIR .. "/plugin_sa/game_sa/*",
        PSDK_DIR .. "/plugin_sa/game_sa/rw/*",
        PSDK_DIR .. "/shared/*",
        PSDK_DIR .. "/shared/rw/*",
        PSDK_DIR .. "/shared/game/*",
    }
    
    libdirs {
        PSDK_DIR .. "/output/lib",
    }

        
    files { 
        "../src/**", 
        "../include/coreutils/imgui/rw/**",
        "../include/coreutils/imgui/fonts/**",
        "../include/imgui/**"
    }

    toolset "clang"
    buildoptions { 
        "--target=i686-w64-mingw32",
        "-std=c++2b", -- C++23 
        "-fpermissive",
        "-fcommon",
        "-fms-extensions",
        "-Wno-microsoft-include",
    }

    linkoptions  { 
        "--target=i686-w64-mingw32",
        "-static",             -- Ensure the whole binary is static
        "-static-libgcc",      -- Link GCC runtime statically
        "-static-libstdc++",   -- This fixes your ios_base_library_init error
        "-pthread",             -- Often required for std::thread/mutex support in MinGW
        "-lstdc++",
        "-Wl,-s",
        "-Wl,--allow-multiple-definition" -- The "Emergency Exit" flag
    }

    cppdialect "C++23"
    
    postbuildcommands {
        "{COPY} \"$(TARGET)\" \"" .. GAME_DIR .. "\""
    }

    filter "configurations:Debug"
        symbols "Off"
        links { 
            "plugin_d",
            "dwmapi",
            "shell32",
            "gdi32",
            "ntdll"
        } 
        
    filter "configurations:Release"
        optimize "Speed"
        symbols "Off"
        links { "plugin" }