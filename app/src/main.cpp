
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

extern "C" // Ensures C linkage - prevents C++ name mangling
{
	// Tells NVIDIA driver to use the dedicated GPU for this application
	__declspec(dllexport) // Exports this symbol so the NVIDIA driver can see it
		unsigned long NvOptimusEnablement = 0x00000001; // Setting to 1 enables dedicated GPU

	__declspec(dllexport)
		int AmdPowerXpressRequestHighPerformance = 1;
}

int main(int argc, char* argv[]) {
	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
	{
		std::cerr << "SDL Init Error\n";
		return -1;
	}

	// Create window with SDL renderer
	SDL_Window* window = SDL_CreateWindow("Encore",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		1200, 800, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	auto io = ImGui::GetIO();

	auto glContext = SDL_GL_CreateContext(window);

	glewInit();

	{
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

		SDL_SetWindowFullscreen(window, false);
		SDL_SetWindowResizable(window, SDL_TRUE);

		SDL_GL_SetSwapInterval(0);
	}

	// have this by default
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0f, 1.0f);

	ImGui_ImplSDL2_InitForOpenGL(window, &glContext);
	ImGui_ImplOpenGL3_Init("#version 130");

	ImGui::StyleColorsDark();

	//
	FrameTimeTracker track;

	float deltaTime = 0.0f;
	float lastFrameNow = 0.0f;

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
				if(event.key.keysym.sym == SDLK_ESCAPE)
				{
					bGameRunning = false;
				}
				break;
			default: break;
			}
		}

		// UPDATE
		track.Update(deltaTime);

		// RENDER

		SDL_GL_MakeCurrent(window, glContext);

		// render
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// render ui
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		track.RenderCompactOverlay();
		track.RenderImGuiWindow();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


		SDL_GL_SwapWindow(window);

	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_GL_DeleteContext(glContext);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}