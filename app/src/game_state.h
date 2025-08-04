#pragma once
#include <imgui/imgui.h>
#include <base_arena.h>

#include <SDL2/SDL.h>
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
	GLuint color_texture = 0;
	GLuint depth_texture = 0;
	i32 fb_width = 800;
	i32 fb_height = 600;

#if ENC_DEBUG
	FrameTimeTracker track;
#endif

	float time = 0.0f;

	bool bShowImgui = true;
	bool bShowDemoWindow = false;
};
