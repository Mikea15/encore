#include "core/core_minimal.h"
#include "globals.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>

#include <GL/glew.h>

#ifdef _DEBUG
#pragma comment(lib, "SDL2maind")
#else
#pragma comment(lib, "SDL2main")
#endif

#include "imgui/imgui.h"
#include "imgui/implot.h"
#include "imgui/backends/imgui_impl_sdl2.h"
#include "imgui/backends/imgui_impl_opengl3.h"

#if ENC_DEBUG
#include "debug/framerate_widget.h"
#include "debug/memory_widget.h"
#include "debug/renderer_widget.h"
#endif

#include "memory/base_pool.h"
#include "renderer/render2d_types.h"
#include "renderer/renderer.h"
#include "renderer/renderer_sprite.h"
#include "renderer/window_handler.h"

#include "game_state.h"
#include "tasks/task_system.h"

#include "integrations/livepp_handler.h"

namespace StubWorkload {
	void math_workload(int iterations)
	{
		volatile double result = 1.0;
		for(int i = 0; i < iterations; i++)
		{
			result = sin(result) * cos(result) + sqrt(fabs(result));
			result = fmod(result, 100.0) + 1.0; // Keep result bounded
		}
	}
}

i32 main(i32 argc, char* argv[])
{
#ifdef USE_LPP
	LivePPHandler lppHandler;
	lppHandler.InitSynchedAgent();
#endif
	
	GameState gameState = CreateDefaultGameState();
	// TODO: Load GameState settings from serialized file

	StringFactory::Init(&gameState.arenas[AT_FRAME]);

	WindowHandler window;
	if(!window.InitWindow(gameState))
	{
		LOG_ERROR("Could not Init Window");
		return -1;
	}

	window.InitImGui();
	window.InitOpenGL(gameState);

	// Rendering Engine
	RenderingEngine renderEngine;
	renderEngine.CreateFramebuffer(gameState);

	// Render Info
	Render2DInfo renderInfo;
	renderInfo.renderer.Init();

	// Init Systems
#if ENC_DEBUG
	frame_stats_init(g_frameStats, gameState.arenas[AT_GLOBAL]);
#endif

	PROFILE_SET_THREAD_NAME("MainThread");

	task::TaskSchedulerSystem taskScheduler;
	for(int i = 0; i < 1000; i++)
	{
		const char* str = StringFactory::Format("Task %d", i);
		taskScheduler.CreateTask(str, []() {
			const u32 randWorkLoad = (rand() % 5000) + 250;
			StubWorkload::math_workload(randWorkLoad); });
	}
	taskScheduler.CreateExecutionPlan();

	// Create some test sprites
	for(int i = 0; i < 10000; ++i)
	{
		Sprite sprite;
		sprite.position = {
			(rand() % 2000) - 1000.0f, // Random X: -1000 to 1000
			(rand() % 2000) - 1000.0f // Random Y: -1000 to 1000
		};
		sprite.size = { (rand() % 32) + 1.0f, (rand() % 32) + 1.0f };
		sprite.color = {
			(rand() % 255) / 255.0f, // Random color
			(rand() % 255) / 255.0f,
			(rand() % 255) / 255.0f,
			1.0f
		};
		renderInfo.sprites.push_back(sprite);
	}

	f32 deltaTime = 0.0f;
	f32 lastUpdate = 0u;

	// Main Loop
	bool bGameRunning = true;
	while(bGameRunning)
	{
#ifdef USE_LPP
		{
			PROFILE_SCOPE("LPP_SyncPoint");
			lppHandler.SyncPoint();
		}
#endif

		PROFILE_FRAME_START_ALL_THREADS();

		// Reset Frame Arena
		ARENA_SAVE(&gameState.arenas[AT_FRAME]);

		// Calculate delta time
		const f32 timeNow = SDL_GetTicks64() / 1000.0f;
		deltaTime = static_cast<f32>(timeNow - lastUpdate);
		lastUpdate = timeNow;

		// INPUT
		Vec2 camMovementInput = { 0.0f, 0.0f };

		// EVENT BASED INPUT
		{
			PROFILE_SCOPE("Input3");

			SDL_Event event;
			while(SDL_PollEvent(&event))
			{
				// Handle Input
				ImGui_ImplSDL2_ProcessEvent(&event);

				switch(event.type)
				{
				case SDL_QUIT:
					bGameRunning = false;
					break;
				case SDL_WINDOWEVENT:
					if(event.window.event == SDL_WINDOWEVENT_CLOSE
						&& event.window.windowID == SDL_GetWindowID(gameState.window.pWindow))
					{
						bGameRunning = false;
					}
					if(event.window.event == SDL_WINDOWEVENT_RESIZED)
					{
						gameState.window.width = event.window.data1;
						gameState.window.height = event.window.data2;
					}
					break;
				case SDL_KEYDOWN:
					if(event.key.keysym.sym == SDLK_TAB)
					{
						gameState.editor.bShowImGui = !gameState.editor.bShowImGui;
					}
					if(event.key.keysym.sym == SDLK_ESCAPE)
					{
						bGameRunning = false;
					}
					break;
				default: break;
				}
			}

			// REALTIME INPUT
			const u8* keyboardState = SDL_GetKeyboardState(nullptr);

			if(keyboardState[SDL_SCANCODE_A]) { camMovementInput.x = 1.0f; }
			if(keyboardState[SDL_SCANCODE_D]) { camMovementInput.x = -1.0f; }
			if(keyboardState[SDL_SCANCODE_S]) { camMovementInput.y = 1.0f; }
			if(keyboardState[SDL_SCANCODE_W]) { camMovementInput.y = -1.0f; }

			if(keyboardState[SDL_SCANCODE_Q])
			{
				renderInfo.camera.zoom = std::min(100.0f, renderInfo.camera.zoom + 1.0f * deltaTime);
			}
			if(keyboardState[SDL_SCANCODE_E])
			{
				renderInfo.camera.zoom = std::max(0.1f, renderInfo.camera.zoom - 1.0f * deltaTime);
			}

			if(camMovementInput != Vec2(0.0f, 0.0f))
			{
				camMovementInput = glm::normalize(camMovementInput);
			}
		}
		const float cameraSpeed = 500.0f; // pixels per second, adjust as needed

		//~INPUT

		{
			// TASK_GRAPH
			PROFILE_SCOPE("Task Graph");
			taskScheduler.ExecuteTaskGraph();
		}

		// UPDATE
		{
			PROFILE_SCOPE("Gameplay Update");
#if ENC_DEBUG
			frame_stats_update(g_frameStats, deltaTime);
#endif

			// Rotate Sprites
			for(u32 i = 0; i < renderInfo.sprites.size(); ++i)
			{
				renderInfo.sprites[i].rotation += deltaTime * rand() * 0.03f;
			}

			{
				PROFILE_SCOPE("Late Camera Update");

				Vec2 targetVelocity = camMovementInput * cameraSpeed;
				renderInfo.camera.cameraVelocity = glm::mix(renderInfo.camera.cameraVelocity, targetVelocity,
					renderInfo.camera.cameraDamping * deltaTime);
				renderInfo.camera.position += renderInfo.camera.cameraVelocity * deltaTime;
			}
		}
		//~UPDATE

		// RENDER
		{
			PROFILE_SCOPE("Render Frame");
			renderEngine.RenderFrame(gameState, renderInfo);
		}
		//~RENDER

		ARENA_RESET(&gameState.arenas[AT_FRAME]);
	}

	renderEngine.ClearFramebuffer(gameState);

#ifdef USE_LPP
	lppHandler.Clear();
#endif

	window.ShutdownImGui();

	renderInfo.renderer.Clear();

	ClearGameState(gameState);

	window.Quit();

	return 0;
}
