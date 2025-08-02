
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>

#include <GL/glew.h>

#ifdef _DEBUG
#pragma comment(lib, "SDL2maind")
#else
#pragma comment(lib, "SDL2main")
#endif

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_sdl2.h"
#include "imgui/backends/imgui_impl_opengl3.h"

#include <iostream>
#include <string>
#include <chrono>

#include "debug/framerate_widget.h"
#include "debug/memory_widget.h"
#include "base_pool.h"

struct GraphicsComponent {
	DECLARE_POOL(GraphicsComponent);

	u8 modelData[1024];
	const char* name;
	bool bActive;
};
IMPLEMENT_POOL(GraphicsComponent, 500);

struct GameState {
	Arena globalArena;
	Arena componentsArena;
	Arena enemiesArena;
	Arena uiArena;

	struct Window {
		SDL_Window* pWindow;
		SDL_GLContext pGLContext;
	} window;
};

int main(int argc, char* argv[]) {

	GameState gameState;
	// Memory
	gameState.globalArena = arena_create(KB(24));
	gameState.componentsArena = arena_create(KB(30));
	gameState.enemiesArena = arena_create(KB(125));
	gameState.uiArena = arena_create(MB(2));

	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
	{
		LOG_ERROR("Failed to SDL_Init");
		return -1;
	}

	// Create window with SDL renderer
	gameState.window.pWindow = SDL_CreateWindow("Encore", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1200, 800, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (!gameState.window.pWindow) {
		LOG_ERROR("Could not Create Window");
		return -1;
	}

	// Init IMGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	// Init OpenGL
	// Create GL Context
	gameState.window.pGLContext = SDL_GL_CreateContext(gameState.window.pWindow);
	ImGui_ImplSDL2_InitForOpenGL(gameState.window.pWindow, &gameState.window.pGLContext);
	ImGui_ImplOpenGL3_Init("#version 130");
	{
		glewInit();

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);

		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

		SDL_SetWindowFullscreen(gameState.window.pWindow, false);
		SDL_SetWindowResizable(gameState.window.pWindow, SDL_TRUE);

		SDL_GL_SetSwapInterval(1);
	}

	// have this by default
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0f, 1.0f);

	// Init Systems
	GraphicsComponent::Init(&gameState.componentsArena);
	test_pool(&gameState.uiArena);
	FrameTimeTracker track(gameState.globalArena);

	float deltaTime = 0.0f;
	float lastFrameNow = 0.0f;

	// Main Loop
	bool bGameRunning = true;
	while (bGameRunning) {
		// Calculate delta time
		const float frameNow = SDL_GetTicks() / 1000.0f;
		deltaTime = frameNow - lastFrameNow;
		lastFrameNow = frameNow;

		// INPUT
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			// Handle Input
			ImGui_ImplSDL2_ProcessEvent(&event);

			switch (event.type)
			{
			case SDL_QUIT:
				bGameRunning = false;
				break;
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE)
				{
					bGameRunning = false;
				}
				break;
			default: break;
			}
		}
		
		//~INPUT

		// UPDATE
		track.Update(deltaTime);
		//~UPDATE

		// RENDER
		SDL_GL_MakeCurrent(gameState.window.pWindow, gameState.window.pGLContext);
		glClearColor(sin(frameNow), cos(frameNow), 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Render Passes Here.

		// render ui
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		ImGui::ShowDemoWindow(); // Show demo window! :)
		track.RenderCompactOverlay();
		track.RenderImGuiWindow();

		debug::DrawMemoryStats(gameState.globalArena, "Global");
		debug::DrawMemoryStats(gameState.componentsArena, "Components");
		debug::DrawMemoryStats(gameState.enemiesArena, "Enemies");
		debug::DrawMemoryStats(gameState.uiArena, "UI");

		ImGui::Render();
		//~render ui
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		SDL_GL_SwapWindow(gameState.window.pWindow);
		//~RENDER
	}


	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_GL_DeleteContext(gameState.window.pGLContext);
	SDL_DestroyWindow(gameState.window.pWindow);
	SDL_Quit();

	return 0;
}
