#pragma once

#include "core/core_minimal.h"
#include "memory/base_arena.h"

#include <SDL2/SDL.h>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_sdl2.h>
#include <GL/glew.h>

#if ENC_DEBUG
#include "debug/framerate_widget.h"
#endif

struct GameState
{
	Arena globalArena;
	Arena componentsArena;
	Arena enemiesArena;
	Arena uiArena;

	struct Window
	{
		SDL_Window* pWindow;
		SDL_GLContext pGLContext;
		ImVec2 viewportSize;
		ImVec2 viewportPos;
		i32 width;
		i32 height;
	} window;

	// Framebuffer for rendering the scene
	GLuint framebuffer = 0;
	GLuint colorTexture = 0;
	GLuint depthTexture = 0;
	i32 framebufferWidth = 800;
	i32 framebufferHeight = 600;

#if ENC_DEBUG
	FrameTimeTracker track;
#endif

	float time = 0.0f;

	bool bShowImgui = true;
	bool bShowDemoWindow = false;
};
