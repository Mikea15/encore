workspace "encore"
    architecture "x64"
    configurations { "Debug", "Profile", "Release" }
    startproject "encore_app"

    -- Workspace-level settings
    flags { 
        "MultiProcessorCompile"
     }
    
    -- Output directories
    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

    -- Version and build number management
    local versionMajor = 1
    local versionMinor = 0
    local versionPatch = 0
    local buildNumber = 0
    
    -- Read existing build number from file
    local buildFile = "build_number.txt"
    if os.isfile(buildFile) then
        local file = io.open(buildFile, "r")
        if file then
            local content = file:read("*a")
            file:close()
            buildNumber = tonumber(content) or 0
        end
    end
    
    -- Increment build number
    buildNumber = buildNumber + 1
    
    -- Write new build number back to file
    local file = io.open(buildFile, "w")
    if file then
        file:write(tostring(buildNumber))
        file:close()
    end
    
    -- Create version string
    local versionString = string.format("%d.%d.%d.%d", versionMajor, versionMinor, versionPatch, buildNumber)
    local buildDate = os.date("%Y-%m-%d")
    local buildTime = os.date("%H:%M:%S")
    local buildTimestamp = os.date("%Y%m%d%H%M%S")
    
    print("Building version: " .. versionString .. " (Build #" .. buildNumber .. ")")

    -- Global defines
    defines
    {
        "GLM_FORCE_DEPTH_ZERO_TO_ONE",  -- Vulkan/DirectX depth convention
        "GLM_FORCE_LEFT_HANDED",  -- Left-handed coordinate system
        -- Version defines
        "VERSION_MAJOR=" .. versionMajor,
        "VERSION_MINOR=" .. versionMinor,
        "VERSION_PATCH=" .. versionPatch,
        "BUILD_NUMBER=" .. buildNumber,
        "VERSION_STRING=\"" .. versionString .. "\"",
        "BUILD_DATE=\"" .. buildDate .. "\"",
        "BUILD_TIME=\"" .. buildTime .. "\"",
        "BUILD_TIMESTAMP=\"" .. buildTimestamp .. "\""
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

newaction {
    trigger = "resetbuild",
    description = "Reset build number to 0",
    execute = function()
        local file = io.open("build_number.txt", "w")
        if file then
            file:write("0")
            file:close()
            print("Build number reset to 0")
        end
    end
}

-- Show version info
newaction {
    trigger = "version",
    description = "Show current version information",
    execute = function()
        local buildNumber = 0
        if os.isfile("build_number.txt") then
            local file = io.open("build_number.txt", "r")
            if file then
                buildNumber = tonumber(file:read("*a")) or 0
                file:close()
            end
        end
        print("Version: 1.0.0." .. buildNumber)
        print("Build Date: " .. os.date("%Y-%m-%d %H:%M:%S"))
    end
}

