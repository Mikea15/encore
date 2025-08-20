workspace "encore"
    architecture "x64"
    configurations { "Debug", "Profile", "Release" }
    startproject "app"

    -- Output directories
    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

    -- vcpkg integration
    filter "system:windows"
        toolset "msc-v143"
        -- Enable vcpkg manifest mode
    filter {}

-- Enable vcpkg manifest mode globally
filter "action:vs*"
    buildoptions { "/p:VcpkgEnableManifest=true" }
filter {}

project "encore_core"
    location "encore_core"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("intermediate/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.c"
    }

    vpaths { ["*"] = "**" }

    includedirs
    {
        "%{prj.name}/src"
    }

    -- vcpkg packages
    filter "system:windows"
        includedirs
        {
            "$(VCPKG_ROOT)/installed/x64-windows/include",
            "vcpkg_installed/x64-windows/include"
        }
        libdirs
        {
            "$(VCPKG_ROOT)/installed/x64-windows/lib",
            "$(VCPKG_ROOT)/installed/x64-windows/debug/lib",
            "vcpkg_installed/x64-windows/lib",
            "vcpkg_installed/x64-windows/debug/lib"
        }
        buildoptions { "/p:VcpkgEnableManifest=true" }
    filter {}

    -- Configuration specific settings
    filter "configurations:Debug"
        defines { "ENC_DEBUG", "ENC_EDITOR", "ENC_IMGUI" }
        runtime "Debug"
        symbols "on"
        optimize "off"

    filter "configurations:Profile"
        defines { "ENC_PROFILE", "ENC_EDITOR", "ENC_IMGUI" }
        runtime "Release"
        symbols "on"
        optimize "speed"

    filter "configurations:Release"
        defines { "ENC_RELEASE" }
        runtime "Release"
        symbols "off"
        optimize "full"

    filter {}

project "encore_app"
    location "encore_app"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("intermediate/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.c",
        "%{prj.name}/vendor/**.h",
        "%{prj.name}/vendor/**.cpp",
        "%{prj.name}/vendor/**.c"
    }

    vpaths { ["*"] = "**" }

    includedirs
    {
        "%{prj.name}/src",
        "%{prj.name}/vendor",
        "encore_core/src"
    }

    links
    {
        "encore_core"
    }

    -- vcpkg packages
    filter "system:windows"
        includedirs
        {
            "$(VCPKG_ROOT)/installed/x64-windows/include",
            "vcpkg_installed/x64-windows/include"
        }
        libdirs
        {
            "$(VCPKG_ROOT)/installed/x64-windows/lib",
            "$(VCPKG_ROOT)/installed/x64-windows/debug/lib",
            "vcpkg_installed/x64-windows/lib",
            "vcpkg_installed/x64-windows/debug/lib"
        }
        links
        {
            "opengl32"
        }
        buildoptions { "/p:VcpkgEnableManifest=true" }
    filter {}

    -- Configuration specific settings
    filter "configurations:Debug"
        defines { "ENC_DEBUG", "ENC_EDITOR", "ENC_IMGUI" }
        runtime "Debug"
        symbols "on"
        optimize "off"
        links
        {
            "SDL2d",
            "SDL2maind"
        }

    filter "configurations:Profile"
        defines { "ENC_PROFILE", "ENC_EDITOR", "ENC_IMGUI" }
        runtime "Release"
        symbols "on"
        optimize "speed"

    filter "configurations:Release"
        defines { "ENC_RELEASE" }
        runtime "Release"
        symbols "off"
        optimize "full"

    filter "configurations:Debug"
        postbuildcommands { '{COPY} "%{wks.location}assets" "%{cfg.targetdir}/assets"' }

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