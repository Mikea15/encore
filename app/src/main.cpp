
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>

#include <GL/glew.h>
#include <glm/common.hpp>

#ifdef _DEBUG
#pragma comment(lib, "SDL2maind")
#else
#pragma comment(lib, "SDL2main")
#endif

#include "imgui/imgui.h"
#include "imgui/implot.h"
#include "imgui/backends/imgui_impl_sdl2.h"
#include "imgui/backends/imgui_impl_opengl3.h"

#include <iostream>
#include <string>
#include <chrono>

#include "debug/framerate_widget.h"
#include "debug/memory_widget.h"
#include "base_pool.h"
#include "renderer/renderer_sprite.h"
#include "debug/renderer_widget.h"

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

	FrameTimeTracker track;

	float time = 0.0f;

	bool bShowImgui = true;
	bool bShowDemoWindow = false;
};



void CreateFramebuffer(GameState& gameState) {
	// Delete existing framebuffer if it exists
	if (gameState.framebuffer) {
		glDeleteFramebuffers(1, &gameState.framebuffer);
		glDeleteTextures(1, &gameState.color_texture);
		glDeleteTextures(1, &gameState.depth_texture);
	}

	// Create framebuffer
	glGenFramebuffers(1, &gameState.framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gameState.framebuffer);

	// Create color texture
	glGenTextures(1, &gameState.color_texture);
	glBindTexture(GL_TEXTURE_2D, gameState.color_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, gameState.fb_width, gameState.fb_height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gameState.color_texture, 0);

	// Create depth texture
	glGenTextures(1, &gameState.depth_texture);
	glBindTexture(GL_TEXTURE_2D, gameState.depth_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, gameState.fb_width, gameState.fb_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gameState.depth_texture, 0);

	// Check framebuffer completeness
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "Framebuffer not complete!" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderSceneGeometry(GameState& gameState) 
{
	// Update time for animation
	gameState.time += 0.016f; // ~60fps

	// Render a rotating colored cube
	glRotatef(gameState.time * 50.0f, 1.0f, 1.0f, 0.0f);

	glBegin(GL_QUADS);

	// Front face (red)
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(-0.5f, -0.5f, 0.5f);
	glVertex3f(0.5f, -0.5f, 0.5f);
	glVertex3f(0.5f, 0.5f, 0.5f);
	glVertex3f(-0.5f, 0.5f, 0.5f);

	// Back face (green)
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-0.5f, -0.5f, -0.5f);
	glVertex3f(-0.5f, 0.5f, -0.5f);
	glVertex3f(0.5f, 0.5f, -0.5f);
	glVertex3f(0.5f, -0.5f, -0.5f);

	// Top face (blue)
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-0.5f, 0.5f, -0.5f);
	glVertex3f(-0.5f, 0.5f, 0.5f);
	glVertex3f(0.5f, 0.5f, 0.5f);
	glVertex3f(0.5f, 0.5f, -0.5f);

	// Bottom face (yellow)
	glColor3f(1.0f, 1.0f, 0.0f);
	glVertex3f(-0.5f, -0.5f, -0.5f);
	glVertex3f(0.5f, -0.5f, -0.5f);
	glVertex3f(0.5f, -0.5f, 0.5f);
	glVertex3f(-0.5f, -0.5f, 0.5f);

	// Right face (magenta)
	glColor3f(1.0f, 0.0f, 1.0f);
	glVertex3f(0.5f, -0.5f, -0.5f);
	glVertex3f(0.5f, 0.5f, -0.5f);
	glVertex3f(0.5f, 0.5f, 0.5f);
	glVertex3f(0.5f, -0.5f, 0.5f);

	// Left face (cyan)
	glColor3f(0.0f, 1.0f, 1.0f);
	glVertex3f(-0.5f, -0.5f, -0.5f);
	glVertex3f(-0.5f, -0.5f, 0.5f);
	glVertex3f(-0.5f, 0.5f, 0.5f);
	glVertex3f(-0.5f, 0.5f, -0.5f);

	glEnd();

	// Render some additional geometry for a more interesting scene
	glPushMatrix();
	glTranslatef(1.5f, 0.0f, 0.0f);
	glRotatef(gameState.time * -30.0f, 0.0f, 1.0f, 0.0f);
	glScalef(0.5f, 0.5f, 0.5f);

	glBegin(GL_TRIANGLES);
	glColor3f(1.0f, 0.5f, 0.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);
	glColor3f(0.5f, 1.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glColor3f(0.0f, 0.5f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);

	glColor3f(1.0f, 0.5f, 0.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);
	glColor3f(0.0f, 0.5f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glColor3f(1.0f, 0.0f, 0.5f);
	glVertex3f(1.0f, -1.0f, -1.0f);

	glColor3f(1.0f, 0.5f, 0.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);
	glColor3f(1.0f, 0.0f, 0.5f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glColor3f(0.5f, 0.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);

	glColor3f(1.0f, 0.5f, 0.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);
	glColor3f(0.5f, 0.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glColor3f(0.5f, 1.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glEnd();

	glPopMatrix();
}

void ResizeFramebuffer(GameState& gameState, i32 width, i32 height) 
{
	if (width != gameState.fb_width || height != gameState.fb_height) 
	{
		gameState.fb_width = width;
		gameState.fb_height = height;
		CreateFramebuffer(gameState);
	}
}

void RenderScene(GameState& gameState, Render2D& render2D)
{
	// In fullscreen mode, determine the proper size for the framebuffer
	if (!gameState.bShowImgui) 
	{
		SDL_GetWindowSize(gameState.window.pWindow, &gameState.window.width, &gameState.window.height);
		ResizeFramebuffer(gameState, gameState.window.width, gameState.window.height);
	}

	// Always render to framebuffer first
	glBindFramebuffer(GL_FRAMEBUFFER, gameState.framebuffer);
	glViewport(0, 0, gameState.fb_width, gameState.fb_height);

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 2D Rendering Setup
	glDisable(GL_DEPTH_TEST); // Disable depth testing for 2D
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	render2D.renderer.Begin(render2D.camera, gameState.fb_width, gameState.fb_height);

	// Draw background
	render2D.renderer.DrawSprite({ 0, 0 }, { 2000, 2000 }, 0, { 0.1f, 0.1f, 0.2f, 1.0f });

	// Draw all sprites
	for (const auto& sprite : render2D.sprites) {
		render2D.renderer.DrawSprite(sprite);
	}

	// Draw UI elements (these don't move with camera)
	// You'd set up a separate UI camera/projection for this

	render2D.renderer.End();

	// 3D
	//// Clear
	//glEnable(GL_DEPTH_TEST);
	//glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//// Set up projection matrix
	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();
	//float aspect = (float)gameState.fb_width / (float)gameState.fb_height;
	//glFrustum(-aspect * 0.1f, aspect * 0.1f, -0.1f, 0.1f, 0.1f, 100.0f);

	//// Set up modelview matrix
	//glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();
	//glTranslatef(0.0f, 0.0f, -3.0f);

	//// Render the actual scene geometry
	//RenderSceneGeometry(gameState);

	// Unbind framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Reset viewport to window size for final presentation
	glViewport(0, 0, gameState.window.width, gameState.window.height);
}

void RenderImGui(GameState& gameState, Render2D& renderer)
{
	if (!gameState.bShowImgui) return;

	// Create a fullscreen dockspace
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
	window_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	ImGui::Begin("DockSpace Demo", nullptr, window_flags);
	ImGui::PopStyleVar(3);

	// Menu bar
	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("View")) {
			ImGui::MenuItem("Toggle ImGui", "TAB", &gameState.bShowImgui);
			ImGui::MenuItem("Demo Window", nullptr, &gameState.bShowDemoWindow);
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Help")) {
			ImGui::Text("TAB: Toggle Editor Mode");
			ImGui::Text("ESC: Exit application");
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	// DockSpace
	ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

	ImGui::End();

	// Left panel
	ImGui::Begin("Properties");
	ImGui::Text("Scene Properties");
	ImGui::Text("Framebuffer Size: %d x %d", gameState.fb_width, gameState.fb_height);
	ImGui::Text("Time: %.2f", time);
	ImGui::Separator();
	ImGui::Text("Controls:");
	ImGui::BulletText("F1: Toggle UI");
	ImGui::BulletText("ESC: Exit");
	if (ImGui::Button("Reset Time")) {
		gameState.time = 0.0f;
	}
	ImGui::End();

	// Right panel
	debug::DrawMemoryStats(gameState.globalArena, "Global");
	debug::DrawMemoryStats(gameState.componentsArena, "Components");
	debug::DrawMemoryStats(gameState.enemiesArena, "Enemies");
	debug::DrawMemoryStats(gameState.uiArena, "UI");

	// Bottom panel
	ImGui::Begin("Console");
	ImGui::Text("Console Output");
	ImGui::Separator();
	ImGui::Text("Application running...");
	ImGui::Text("Viewport mode: %s", gameState.bShowImgui ? "Editor" : "Fullscreen");
	ImGui::Text("Scene rendering to texture: %dx%d", gameState.fb_width, gameState.fb_height);
	ImGui::End();

	// PerfPanel
	gameState.track.RenderImGuiWindow();

	// Rendering
	debug::DrawRendererStats(renderer);

	// Central viewport window - this displays the 3D scene
	ImGui::Begin("Scene Viewport");

	// Get the content region size
	ImVec2 content_region = ImGui::GetContentRegionAvail();
	i32 new_width = (i32)content_region.x;
	i32 new_height = (i32)content_region.y;

	// Ensure minimum size
	if (new_width < 64) new_width = 64;
	if (new_height < 64) new_height = 64;

	// Resize framebuffer if needed
	ResizeFramebuffer(gameState, new_width, new_height);

	// Display the rendered scene texture
	ImGui::Image((void*)(intptr_t)gameState.color_texture, 
		ImVec2(gameState.fb_width, gameState.fb_height), ImVec2(0, 1), ImVec2(1, 0));

	ImGui::End();

	// Demo window
	if (gameState.bShowDemoWindow) {
		ImGui::ShowDemoWindow(&gameState.bShowDemoWindow);
	}
}

void BlitFramebufferToScreen(GameState& gameState) {
	// Blit the framebuffer to the screen in fullscreen mode
	SDL_GetWindowSize(gameState.window.pWindow, &gameState.window.width, &gameState.window.height);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, gameState.framebuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	glBlitFramebuffer(
		0, 0, gameState.fb_width, gameState.fb_height,  // src rect
		0, 0, gameState.window.width, gameState.window.height,  // dst rect
		GL_COLOR_BUFFER_BIT, GL_LINEAR
	);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


i32 main(i32 argc, char* argv[]) {

	GameState gameState;
	// Memory
	gameState.globalArena = arena_create(KB(24));
	gameState.componentsArena = arena_create(KB(30));
	gameState.enemiesArena = arena_create(MB(3));
	gameState.uiArena = arena_create(MB(2));

	gameState.window.width = 1280;
	gameState.window.height = 720;

	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
	{
		LOG_ERROR("Failed to SDL_Init");
		return -1;
	}

	// Create window with SDL renderer
	gameState.window.pWindow = SDL_CreateWindow("Encore", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
		gameState.window.width, gameState.window.height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	if (!gameState.window.pWindow) {
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
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

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

	// Create framebuffer for scene rendering
	CreateFramebuffer(gameState);

	// Init Systems
	gameState.track.Init(gameState.globalArena);

	Render2D render2D;

	// Init Renderer
	{
		// Create some test sprites
		for (int i = 0; i < 100000; ++i) {
			Sprite sprite;
			sprite.position = {
				(rand() % 2000) - 1000.0f,  // Random X: -1000 to 1000
				(rand() % 2000) - 1000.0f   // Random Y: -1000 to 1000
			};
			sprite.size = { (rand() % 32) + 1.0f, (rand() % 32) + 1.0f };
			sprite.color = {
				(rand() % 255) / 255.0f,    // Random color
				(rand() % 255) / 255.0f,
				(rand() % 255) / 255.0f,
				1.0f
			};
			render2D.sprites.push_back(sprite);
		}
	}

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
		// EVENT BASED INPUT
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
			case SDL_WINDOWEVENT:
				if (event.window.event == SDL_WINDOWEVENT_CLOSE 
					&& event.window.windowID == SDL_GetWindowID(gameState.window.pWindow))
				{
					bGameRunning = false;
				}
				if(event.window.event == SDL_WINDOWEVENT_RESIZED) {
					gameState.window.width = event.window.data1;
					gameState.window.height = event.window.data2;
				}
				break;
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_TAB)
				{
					gameState.bShowImgui = !gameState.bShowImgui;
				}
				if (event.key.keysym.sym == SDLK_ESCAPE)
				{
					bGameRunning = false;
				}
				break;
			default: break;
			}
		}

		// REALTIME INPUT
		const u8* keyboardState = SDL_GetKeyboardState(nullptr);

		Vec2 camMovementInput = { 0.0f, 0.0f };
		if (keyboardState[SDL_SCANCODE_A]) { camMovementInput.x = 1.0f; }
		if (keyboardState[SDL_SCANCODE_D]) { camMovementInput.x = -1.0f; }
		if (keyboardState[SDL_SCANCODE_S]) { camMovementInput.y = 1.0f; }
		if (keyboardState[SDL_SCANCODE_W]) { camMovementInput.y = -1.0f; }
		
		if (keyboardState[SDL_SCANCODE_Q]) {
			render2D.camera.zoom = std::min(100.0f, render2D.camera.zoom + 1.0f * deltaTime);
		}
		if (keyboardState[SDL_SCANCODE_E]) {
			render2D.camera.zoom = std::max(0.1f, render2D.camera.zoom - 1.0f * deltaTime);
		}
		
		if (camMovementInput != Vec2(0.0f, 0.0f)) {
			camMovementInput = glm::normalize(camMovementInput);
		}

		const float cameraSpeed = 500.0f; // pixels per second, adjust as needed
			
		Vec2 targetVelocity = camMovementInput * cameraSpeed;
		render2D.camera.cameraVelocity = glm::mix(render2D.camera.cameraVelocity, targetVelocity, render2D.camera.cameraDamping * deltaTime);
		render2D.camera.position += render2D.camera.cameraVelocity * deltaTime;

		//~INPUT

		// UPDATE
		gameState.track.Update(deltaTime);

		// Rotate Sprites
		for (u32 i = 0; i < render2D.sprites.size(); ++i) {
			render2D.sprites[i].rotation += deltaTime * rand() * 0.03f;
		}
		//~UPDATE

		// RENDER
		// render ui
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		// Always render the scene once to the framebuffer
		RenderScene(gameState, render2D);

		if (gameState.bShowImgui)
		{
			// In editor mode: display the texture in ImGui
			RenderImGui(gameState, render2D);
		}
		else 
		{
			gameState.track.RenderCompactOverlay();

			// In fullscreen mode: blit the framebuffer to screen
			BlitFramebufferToScreen(gameState);
		}

		ImGui::Render();
		//~render ui

		if (gameState.bShowImgui)
		{
			// Clear the screen and render ImGui
			SDL_GetWindowSize(gameState.window.pWindow, &gameState.window.width, &gameState.window.height);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, gameState.window.width, gameState.window.height);
			glClearColor(sin(frameNow), cos(frameNow), 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
		}

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Update and Render additional Platform Windows
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
			SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
		}

		SDL_GL_SwapWindow(gameState.window.pWindow);
		//~RENDER
	}

	if (gameState.framebuffer) {
		glDeleteFramebuffers(1, &gameState.framebuffer);
		glDeleteTextures(1, &gameState.color_texture);
		glDeleteTextures(1, &gameState.depth_texture);
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImPlot::DestroyContext();
	ImGui::DestroyContext();

	SDL_GL_DeleteContext(gameState.window.pGLContext);
	SDL_DestroyWindow(gameState.window.pWindow);

	arena_destroy(&gameState.globalArena);
	arena_destroy(&gameState.componentsArena);
	arena_destroy(&gameState.enemiesArena);
	arena_destroy(&gameState.uiArena);

	SDL_Quit();

	return 0;
}
