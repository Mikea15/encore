#include "rendering_engine.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui/backends/imgui_impl_sdl2.h"

#include "debug/framerate_widget.h"
#include <utils/utils_math.h>

RenderingEngine::RenderingEngine(SpriteBatchRenderer& renderer)
	: m_2dRenderer(renderer)
{}

void RenderingEngine::Init(GameState & rGameState)
{
	m_2dRenderer.Init();

	m_renderCommands.reserve(100'000);

	CreateFramebuffer(rGameState);

	m_imguiRenderer.RegisterWidget(new ImGuiMenu());
	m_imguiRenderer.RegisterWidget(new ImGuiMemoryMonitor());
}

void RenderingEngine::Shutdown(GameState& rGameState)
{
	m_2dRenderer.Clear();
	ClearFramebuffer(rGameState);
}

void RenderingEngine::RenderFrame(GameState& gameState, Camera2D& camera)
{
	PROFILE();

	// Always render the scene once to the framebuffer
	RenderScene(gameState, camera);

	// In fullscreen mode: blit the framebuffer to screen
	BlitFramebufferToScreen(gameState);

	{
		PROFILE_SCOPE("SwapWindow");
		SDL_GL_SwapWindow(gameState.window.pWindow);
	}
}

void RenderingEngine::RenderEditorFrame(GameState& gameState, Camera2D& camera)
{
	PROFILE();

	// Setup ImGui Rendering Frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();

	// In editor mode: display the texture in ImGui
	{
		PROFILE_SCOPE("Render::ImGui Widgets");
		if(!gameState.editor.bShowImGui) return;

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

		// DockSpace
		ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
		
		m_imguiRenderer.Render(gameState);
		
		if(gameState.editor.bOpenProfiler)
		{
			PROFILE_SCOPE("Profiler");
			m_profilerWindow.DrawProfilerFlameGraph(gameState);
		}

		// Bottom panel
		if(ImGui::Begin("Console"))
		{
			PROFILE_SCOPE("Console");
			ImGui::Text("Console Output");
			ImGui::Separator();
			ImGui::Text("Application running...");
			ImGui::Text("Viewport mode: %s", gameState.editor.bShowImGui ? "Editor" : "Fullscreen");
			ImGui::Text("Scene rendering to texture: %dx%d", gameState.framebufferWidth, gameState.framebufferHeight);
		}
		ImGui::End();

		// PerfPanel
#if ENC_DEBUG
		if(gameState.editor.bOpenPerformanceMonitor)
		{
			debug::DrawFrameStats(gameState, g_frameStats);
		}
#endif

		// Rendering
		debug::DrawRendererStats(m_2dRenderer);

		// Central viewport window - this displays the 3D scene
		if(ImGui::Begin("Scene Viewport"))
		{
			PROFILE_SCOPE("Scene ViewPort");
			ImVec2 content_region = ImGui::GetContentRegionAvail();
			content_region.x = utils::Max(64.0f, content_region.x);
			content_region.y = utils::Max(64.0f, content_region.y);

			// Resize framebuffer if needed
			ResizeFramebuffer(gameState, content_region.x, content_region.y);

			// Display the rendered scene texture
			ImGui::Image((void*)(intptr_t)gameState.colorTexture,
				ImVec2((f32)gameState.framebufferWidth, (f32)gameState.framebufferHeight), ImVec2(0, 1), ImVec2(1, 0));

		}
		ImGui::End();

		// Demo window
		if(gameState.editor.bShowDemoWindow)
		{
			ImGui::ShowDemoWindow(&gameState.editor.bShowDemoWindow);
		}

		ImGui::End();
	}

	// Prepare ImGui for Rendering
	{
		PROFILE_SCOPE("Render::ImGui::PrepareForRender");
		ImGui::Render();
	}

	// Always render the scene once to the framebuffer
	RenderScene(gameState, camera);

	{
		PROFILE_SCOPE("Render::Blit");
		// Clear the screen and render ImGui
		SDL_GetWindowSize(gameState.window.pWindow, &gameState.window.width, &gameState.window.height);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, gameState.window.width, gameState.window.height);
		glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	{
		PROFILE_SCOPE("Render::ImGui");
		// Draw ImGui on Top
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	// Swap Buffers
	{
		PROFILE_SCOPE("SwapWindow");
		SDL_GL_SwapWindow(gameState.window.pWindow);
	}
}

void RenderingEngine::PushRenderCommand(RenderCommand cmd)
{
	m_renderCommands.push_back(cmd);
}

void RenderingEngine::ClearRenderCommands()
{
	m_renderCommands.clear();
}

void RenderingEngine::RenderScene(GameState& gameState, Camera2D& camera)
{
	PROFILE();

	// In full screen mode, determine the proper size for the framebuffer
	if(!gameState.editor.bShowImGui)
	{
		SDL_GetWindowSize(gameState.window.pWindow, &gameState.window.width, &gameState.window.height);
		ResizeFramebuffer(gameState, gameState.window.width, gameState.window.height);
	}

	// Always render to framebuffer first
	glBindFramebuffer(GL_FRAMEBUFFER, gameState.framebuffer);
	glViewport(0, 0, gameState.framebufferWidth, gameState.framebufferHeight);

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 2D Rendering Setup
	glDisable(GL_DEPTH_TEST); // Disable depth testing for 2D
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_2dRenderer.Begin(camera, (f32)gameState.framebufferWidth, (f32)gameState.framebufferHeight);

	// Draw background
	m_2dRenderer.DrawSprite({ 0, 0 }, { 2000, 2000 }, 0, { 0.1f, 0.1f, 0.2f, 1.0f });

	// Draw all sprites
	for(const RenderCommand& cmd : m_renderCommands)
	{
		m_2dRenderer.DrawSprite(cmd.position, cmd.rotation, cmd.sprite);
	}

	// Draw UI elements (these don't move with camera)
	// You'd set up a separate UI camera/projection for this

	m_2dRenderer.End();

	// Unbind framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Reset viewport to window size for final presentation
	glViewport(0, 0, gameState.window.width, gameState.window.height);
}

void RenderingEngine::BlitFramebufferToScreen(GameState& gameState)
{
	PROFILE();
	// Blit the framebuffer to the screen in fullscreen mode
	SDL_GetWindowSize(gameState.window.pWindow, &gameState.window.width, &gameState.window.height);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, gameState.framebuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	glBlitFramebuffer(
		0, 0, gameState.framebufferWidth, gameState.framebufferHeight,  // src rect
		0, 0, gameState.window.width, gameState.window.height,  // dst rect
		GL_COLOR_BUFFER_BIT, GL_LINEAR
	);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderingEngine::CreateFramebuffer(GameState& gameState)
{
	PROFILE();
	// Delete existing framebuffer if it exists
	if(gameState.framebuffer)
	{
		glDeleteFramebuffers(1, &gameState.framebuffer);
		glDeleteTextures(1, &gameState.colorTexture);
		glDeleteTextures(1, &gameState.depthTexture);
	}

	// Create framebuffer
	glGenFramebuffers(1, &gameState.framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gameState.framebuffer);

	// Create color texture
	glGenTextures(1, &gameState.colorTexture);
	glBindTexture(GL_TEXTURE_2D, gameState.colorTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, gameState.framebufferWidth, gameState.framebufferHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gameState.colorTexture, 0);

	// Create depth texture
	glGenTextures(1, &gameState.depthTexture);
	glBindTexture(GL_TEXTURE_2D, gameState.depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, gameState.framebufferWidth, gameState.framebufferHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gameState.depthTexture, 0);

	// Check framebuffer completeness
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		LOG_ERROR("Framebuffer not complete!");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderingEngine::ClearFramebuffer(GameState& gameState)
{
	if(gameState.framebuffer)
	{
		glDeleteFramebuffers(1, &gameState.framebuffer);
		glDeleteTextures(1, &gameState.colorTexture);
		glDeleteTextures(1, &gameState.depthTexture);
	}
}

void RenderingEngine::ResizeFramebuffer(GameState& gameState, i32 width, i32 height)
{
	if(width != gameState.framebufferWidth || height != gameState.framebufferHeight)
	{
		gameState.framebufferWidth = width;
		gameState.framebufferHeight = height;
		CreateFramebuffer(gameState);
	}
}
