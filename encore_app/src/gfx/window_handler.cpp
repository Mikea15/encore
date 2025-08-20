#include "window_handler.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <GL/glew.h>

#include "imgui/imgui.h"
#include "imgui/implot.h"
#include "imgui/backends/imgui_impl_sdl2.h"
#include "imgui/backends/imgui_impl_opengl3.h"

#include "game_state.h"

#ifdef _DEBUG
#pragma comment(lib, "SDL2maind")
#else
#pragma comment(lib, "SDL2main")
#endif

bool WindowHandler::InitWindow(GameState& gameState)
{
	// Initialize SDL
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
	{
		LOG_ERROR("Failed to SDL_Init");
		return false;
	}

	// Create window with SDL renderer
	gameState.window.pWindow = SDL_CreateWindow("Encore",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		gameState.window.width, gameState.window.height,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	if(!gameState.window.pWindow)
	{
		LOG_ERROR("Could not Create Window");
		return false;
	}

	return true;
}

void WindowHandler::InitImGui()
{
	// Init IMGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImPlot::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();
	if(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
}

void WindowHandler::InitOpenGL(GameState& gameState)
{
	// Create GL Context
	gameState.window.pGLContext = SDL_GL_CreateContext(gameState.window.pWindow);
	ImGui_ImplSDL2_InitForOpenGL(gameState.window.pWindow, &gameState.window.pGLContext);

	ImGui_ImplOpenGL3_Init("#version 130");

	glewInit();

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

	SDL_SetWindowFullscreen(gameState.window.pWindow, false);
	SDL_SetWindowResizable(gameState.window.pWindow, SDL_TRUE);

	SDL_GL_SetSwapInterval(1);

	// have this by default
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0f, 1.0f);
}

void WindowHandler::HandleInputs(const SDL_Event& event)
{
	
}

void WindowHandler::ShutdownImGui()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImPlot::DestroyContext();
	ImGui::DestroyContext();
}

void WindowHandler::ShutdownOpenGL(GameState& gameState)
{
	SDL_GL_DeleteContext(gameState.window.pGLContext);
	SDL_DestroyWindow(gameState.window.pWindow);
}

void WindowHandler::Quit()
{
	SDL_Quit();
}
