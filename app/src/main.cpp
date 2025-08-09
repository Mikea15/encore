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

	// 2. Memory allocation and manipulation
	void memory_workload(int size_kb)
	{
		char* buffer = (char*)malloc(size_kb * 1024);
		if(!buffer) return;

		// Write pattern to memory
		for(int i = 0; i < size_kb * 1024; i++)
		{
			buffer[i] = (char)(i % 256);
		}

		// Read back and modify
		volatile int checksum = 0;
		for(int i = 0; i < size_kb * 1024; i++)
		{
			checksum += buffer[i];
			buffer[i] ^= 0xAA;
		}

		free(buffer);
	}

	// 3. String processing workload
	void string_workload(int string_count)
	{
		char temp[256];
		volatile u64 total_len = 0;

		for(int i = 0; i < string_count; i++)
		{
			snprintf(temp, sizeof(temp), "Processing string number %d with data", i);

			// Simulate string operations
			u64 len = strlen(temp);
			for(int j = 0; j < len; j++)
			{
				temp[j] = (temp[j] >= 'a' && temp[j] <= 'z') ? temp[j] - 32 : temp[j];
			}
			total_len += len;
		}
	}

	// 4. Array sorting workload
	void sorting_workload(int array_size)
	{
		int* arr = (int*)malloc(array_size * sizeof(int));
		if(!arr) return;

		// Fill with random-ish data
		for(int i = 0; i < array_size; i++)
		{
			arr[i] = (i * 17 + 23) % 1000;
		}

		// Simple bubble sort (intentionally inefficient for CPU load)
		for(int i = 0; i < array_size - 1; i++)
		{
			for(int j = 0; j < array_size - i - 1; j++)
			{
				if(arr[j] > arr[j + 1])
				{
					int temp = arr[j];
					arr[j] = arr[j + 1];
					arr[j + 1] = temp;
				}
			}
		}

		free(arr);
	}

	// 5. Hash computation workload
	void hash_workload(int iterations)
	{
		volatile unsigned int hash = 5381;
		char data[64];

		for(int i = 0; i < iterations; i++)
		{
			snprintf(data, sizeof(data), "hash_data_%d_iteration", i);

			// Simple hash computation (djb2 algorithm)
			hash = 5381;
			for(int j = 0; data[j]; j++)
			{
				hash = ((hash << 5) + hash) + data[j];
			}

			// Use hash to prevent optimization
			hash = hash ^ (hash >> 16);
		}
	}
}

struct GraphicsComponent
{
	DECLARE_POOL(GraphicsComponent);

	u8 modelData[1024];
	const char* name;
	bool bActive;
};

IMPLEMENT_POOL(GraphicsComponent, 500);

i32 main(i32 argc, char* argv[])
{
#ifdef USE_LPP
	LivePPHandler lppHandler;
	lppHandler.InitSynchedAgent();
#endif

	GameState gameState;
	// Memory
	gameState.globalArena = arena_create(KILOBYTES(24));
	gameState.componentsArena = arena_create(KILOBYTES(30));
	gameState.enemiesArena = arena_create(MEGABYTES(3));
	gameState.uiArena = arena_create(MEGABYTES(2));
	gameState.frame = arena_create(MEGABYTES(1));

	StringFactory::Init(&gameState.frame);

	gameState.window.width = 1280;
	gameState.window.height = 720;

	// Initialize SDL
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
	{
		LOG_ERROR("Failed to SDL_Init");
		return -1;
	}

	// Create window with SDL renderer
	gameState.window.pWindow = SDL_CreateWindow("Encore", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		gameState.window.width, gameState.window.height,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	if(!gameState.window.pWindow)
	{
		LOG_ERROR("Could not Create Window");
		return -1;
	}

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

	// Init OpenGL
	// Create GL Context
	gameState.window.pGLContext = SDL_GL_CreateContext(gameState.window.pWindow);
	ImGui_ImplSDL2_InitForOpenGL(gameState.window.pWindow, &gameState.window.pGLContext);
	ImGui_ImplOpenGL3_Init("#version 130");
	{
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
	}

	// have this by default
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0f, 1.0f);

	Renderer renderer;
	// Create framebuffer for scene rendering
	renderer.CreateFramebuffer(gameState);

	// Init Systems
#if ENC_DEBUG
	frame_stats_init(g_frameStats, gameState.globalArena);
#endif

	PROFILE_SET_THREAD_NAME("MainThread");

	Render2D render2D;
	render2D.renderer.Init();

	task::OptimizedTaskSystem opTaskSystem2;
	{
		for(int i = 0; i < 1000; i++)
		{
			const char* str = StringFactory::Format("Task %d", i);
			opTaskSystem2.CreateTask(str, []() {
				const u32 randWorkLoad = (rand() % 5000) + 250;
				StubWorkload::math_workload(randWorkLoad); });
		}

		opTaskSystem2.CreateExecutionPlan();
	}

	// Init Renderer
	{
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
			render2D.sprites.push_back(sprite);
		}
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
		ARENA_SAVE(&gameState.frame);

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
				render2D.camera.zoom = std::min(100.0f, render2D.camera.zoom + 1.0f * deltaTime);
			}
			if(keyboardState[SDL_SCANCODE_E])
			{
				render2D.camera.zoom = std::max(0.1f, render2D.camera.zoom - 1.0f * deltaTime);
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
			opTaskSystem2.ExecuteTaskGraph();
		}

		// UPDATE
		{
			PROFILE_SCOPE("Gameplay Update");
#if ENC_DEBUG
			frame_stats_update(g_frameStats, deltaTime);
#endif

			// Rotate Sprites
			for(u32 i = 0; i < render2D.sprites.size(); ++i)
			{
				render2D.sprites[i].rotation += deltaTime * rand() * 0.03f;
			}

			{
				PROFILE_SCOPE("Late Camera Update");

				Vec2 targetVelocity = camMovementInput * cameraSpeed;
				render2D.camera.cameraVelocity = glm::mix(render2D.camera.cameraVelocity, targetVelocity,
					render2D.camera.cameraDamping * deltaTime);
				render2D.camera.position += render2D.camera.cameraVelocity * deltaTime;
			}
		}
		//~UPDATE

		// RENDER
		{
			PROFILE_SCOPE("Render Frame");

			// render ui
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplSDL2_NewFrame();
			ImGui::NewFrame();

			// Always render the scene once to the framebuffer
			renderer.RenderScene(gameState, render2D);

			if (gameState.bShowImgui)
			{
				// In editor mode: display the texture in ImGui
				renderer.RenderImGui(gameState, render2D);
			}
			else
			{
#if ENC_DEBUG
				debug::DrawFrameStatsCompact(g_frameStats);
#endif

				// In fullscreen mode: blit the framebuffer to screen
				renderer.BlitFramebufferToScreen(gameState);
			}

			ImGui::Render();
			//~render ui

			if (gameState.bShowImgui)
			{
				// Clear the screen and render ImGui
				SDL_GetWindowSize(gameState.window.pWindow, &gameState.window.width, &gameState.window.height);
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				glViewport(0, 0, gameState.window.width, gameState.window.height);
				glClearColor(sin(timeNow), cos(timeNow), 0.3f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT);
			}

			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			// Update and Render additional Platform Windows
			if(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
				SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
				SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
			}

			SDL_GL_SwapWindow(gameState.window.pWindow);
			//~RENDER
		}

		ARENA_RESET(&gameState.frame);
	}

	if(gameState.framebuffer)
	{
		glDeleteFramebuffers(1, &gameState.framebuffer);
		glDeleteTextures(1, &gameState.colorTexture);
		glDeleteTextures(1, &gameState.depthTexture);
	}

#ifdef USE_LPP
	lppHandler.Clear();
#endif

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImPlot::DestroyContext();
	ImGui::DestroyContext();

	render2D.renderer.Clear();

	SDL_GL_DeleteContext(gameState.window.pGLContext);
	SDL_DestroyWindow(gameState.window.pWindow);

	arena_destroy(&gameState.globalArena);
	arena_destroy(&gameState.componentsArena);
	arena_destroy(&gameState.enemiesArena);
	arena_destroy(&gameState.uiArena);
	arena_destroy(&gameState.frame);

	SDL_Quit();

	return 0;
}
