#pragma once

#include "core/core_minimal.h"
#include "memory/base_arena.h"

#include <SDL2/SDL.h>"
#include <imgui/imgui.h>
#include <GL/glew.h>

enum eArenaTypes
{
	AT_GLOBAL = 0,
	AT_COMPONENTS,
	AT_FRAME,

	AT_COUNT
};

struct GameState
{
	// Memory Arenas
	Arena arenas[AT_COUNT];

	// Window Handling
	struct Window
	{
		SDL_Window* pWindow;
		SDL_GLContext pGLContext;
		ImVec2 viewportSize;
		ImVec2 viewportPos;
		i32 width;
		i32 height;
	} window;

	// Editor Tools
	struct Editor
	{
		bool bOpenProfiler = true;
		bool bOpenPerformanceMonitor = true;
		bool bOpenMemoryMonitor = true;
		bool bShowImGui = true;
		bool bShowDemoWindow = false;
	} editor;

	// Rendering
	GLuint framebuffer = 0;
	GLuint colorTexture = 0;
	GLuint depthTexture = 0;
	i32 framebufferWidth = 800;
	i32 framebufferHeight = 600;

	// Global
	bool bShowInGameImGui = true;
};
