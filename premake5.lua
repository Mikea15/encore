workspace "encore"
    architecture "x64"
    configurations { "Debug", "Profile", "Release" }
    startproject "encore_app"

    -- Workspace-level settings
    warnings "extra"
    flags { "MultiProcessorCompile" }
    
    -- Output directories
    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

    -- Global defines
    defines
    {
        "GLM_FORCE_DEPTH_ZERO_TO_ONE",  -- Vulkan/DirectX depth convention
        "GLM_FORCE_LEFT_HANDED"  -- Left-handed coordinate system
    }

    -- vcpkg integration
    filter "system:windows"
        toolset "msc-v143"
        -- Enable vcpkg manifest mode
        externalanglebrackets "on"
        externalwarnings "off"
        -- Windows-specific optimizations
        buildoptions { "/bigobj" }  -- For large object files
        linkoptions { "/SUBSYSTEM:CONSOLE" }
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
        defines { "ENC_DEBUG", "ENC_EDITOR", "ENC_IMGUI", "_DEBUG" }
        runtime "Debug"
        symbols "on"
        optimize "off"
        inlining "disabled"
        -- eventually if remove LivePP
        -- editandcontinue "on"  -- Hot reload support

    filter "configurations:Profile"
        defines { "ENC_PROFILE", "ENC_EDITOR", "ENC_IMGUI", "NDEBUG" }
        runtime "Release"
        symbols "on"
        optimize "speed"
        inlining "auto"
        editandcontinue "off"

    filter "configurations:Release"
        defines { "ENC_RELEASE", "NDEBUG" }
        runtime "Release"
        symbols "off"
        optimize "full"
        inlining "auto"
        editandcontinue "off"

    -- Compiler-specific optimizations
    filter { "configurations:Release", "system:windows" }
        buildoptions { "/GL" }  -- Whole program optimization
        linkoptions { "/LTCG" }  -- Link-time code generation

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

newaction {
    trigger = "package",
    description = "Package release build for distribution",
    execute = function()
        print("Creating distribution package...")
        local outputPath = "dist/encore-" .. os.date("%Y%m%d")
        os.mkdir(outputPath)
        os.copyfile("bin/Release-windows-x86_64/encore_app/encore_app.exe", outputPath .. "/encore.exe")
        os.execute('xcopy "assets" "' .. outputPath .. '/assets" /E /I /Y')
        os.copyfile("README.md", outputPath .. "/README.md")
        print("Package created in: " .. outputPath)
    end
}

-- Format code using clang-format
newaction {
    trigger = "format",
    description = "Format all source code using clang-format",
    execute = function()
        print("Formatting code...")
        os.execute("clang-format -i -style=file encore_core/src/**/*.cpp encore_core/src/**/*.h")
        os.execute("clang-format -i -style=file encore_app/src/**/*.cpp encore_app/src/**/*.h")
        print("Code formatting complete!")
    end
}

-- Run static analysis with clang-tidy
newaction {
    trigger = "analyze",
    description = "Run static analysis with clang-tidy",
    execute = function()
        print("Running static analysis...")
        os.execute("clang-tidy encore_core/src/**/*.cpp encore_app/src/**/*.cpp")
        print("Analysis complete!")
    end
}