workspace "Encore"
    architecture "x64"
    configurations { "Debug", "Profile", "Release" }
    startproject "App"

    -- Output directories
    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

    -- vcpkg integration
    filter "system:windows"
        toolset "msc-v143"
    filter {}

project "Core"
    location "core"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/include/**.h"
    }

    includedirs
    {
        "%{prj.name}/src",
        "%{prj.name}/vendor"
    }

    -- vcpkg packages
    filter "system:windows"
        includedirs
        {
            "$(VCPKG_ROOT)/installed/x64-windows/include"
        }
        libdirs
        {
            "$(VCPKG_ROOT)/installed/x64-windows/lib",
            "$(VCPKG_ROOT)/installed/x64-windows/debug/lib"
        }
    filter {}

    -- Configuration specific settings
    filter "configurations:Debug"
        defines { "ENC_DEBUG", "ENC_EDITOR" }
        runtime "Debug"
        symbols "on"
        optimize "off"

    filter "configurations:Profile"
        defines { "ENC_PROFILE", "ENC_EDITOR" }
        runtime "Release"
        symbols "on"
        optimize "speed"

    filter "configurations:Release"
        defines { "ENC_RELEASE" }
        runtime "Release"
        symbols "off"
        optimize "full"

    filter {}

project "App"
    location "app"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp"
    }

    includedirs
    {
        "%{prj.name}/src",
        "core/src"
    }

    links
    {
        "Core"
    }

    -- vcpkg packages
    filter "system:windows"
        includedirs
        {
            "$(VCPKG_ROOT)/installed/x64-windows/include"
        }
        libdirs
        {
            "$(VCPKG_ROOT)/installed/x64-windows/lib",
            "$(VCPKG_ROOT)/installed/x64-windows/debug/lib"
        }
        links
        {
            "SDL2",
            "SDL2main",
            "glew32",
            "opengl32",
            "glm"
        }
    filter {}

    -- Configuration specific settings
    filter "configurations:Debug"
        defines { "ENC_DEBUG", "ENC_EDITOR" }
        runtime "Debug"
        symbols "on"
        optimize "off"
        links
        {
            "SDL2d",
            "SDL2maind"
        }

    filter "configurations:Profile"
        defines { "ENC_PROFILE", "ENC_EDITOR" }
        runtime "Release"
        symbols "on"
        optimize "speed"

    filter "configurations:Release"
        defines { "ENC_RELEASE" }
        runtime "Release"
        symbols "off"
        optimize "full"

    filter {}

-- Platform specific settings
filter "system:windows"
    systemversion "latest"
    defines
    {
        "WIN32_LEAN_AND_MEAN",
        "_CRT_SECURE_NO_WARNINGS"
    }

filter {}