PSDK_DIR = os.getenv("PLUGIN_SDK_DIR")

function addIncludeDirs(root)
    for _, dir in ipairs(os.matchdirs(root .. "/*")) do
        includedirs { dir }
        addIncludeDirs(dir)
    end
end

workspace "ModelExtras"
    configurations { "Debug", "Release" }
    architecture "x86"
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
        "-std=c++2b", -- C++23 
        "--target=i686-w64-mingw32",
        "-fpermissive",
        "-fcommon",
        "-fms-extensions",
        "-Wno-microsoft-include",
        "-static"
        -- "-static-libgcc",
        -- "-static-libstdc++" 
    }

    cppdialect "C++23"
    

    filter "configurations:Debug"
        symbols "On"
        links { 
            "plugin_d",
            "dwmapi",
            "ntdll",
            "winpthread",
            "shell32",
            "gdi32"
        } 
        
    filter "configurations:Release"
        optimize "Speed"
        symbols "On"
        links { "plugin" }