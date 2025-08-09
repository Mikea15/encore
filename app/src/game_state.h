#pragma once

#include "core/core_minimal.h"
#include "memory/base_arena.h"

#include <SDL2/SDL.h>
#include <imgui/imgui.h>
#include <GL/glew.h>

struct GameState
{
	// Memory Arenas
	Arena globalArena;
	Arena componentsArena;
	Arena enemiesArena;
	Arena uiArena;
	Arena frame;

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
