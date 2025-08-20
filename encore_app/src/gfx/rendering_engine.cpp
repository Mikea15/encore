
#include "rendering_engine.h"

#include "debug/framerate_widget.h"
#include "debug/renderer_widget.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui/backends/imgui_impl_sdl2.h"
#include "profiler/profiler.h"
#include "profiler/profiler_section.h"
#include "utils/utils_math.h"


RenderingEngine::RenderingEngine()
	: m_spriteRenderer()
{
}

void RenderingEngine::Init(GameState& rGameState)
{
	m_spriteRenderer.Init();

	m_renderCommands.reserve(100'000);

	CreateFramebuffer(rGameState);

	m_imguiRenderer.RegisterWidget(new ImGuiMenu());
	m_imguiRenderer.RegisterWidget(new ImGuiMemoryMonitor());
}

void RenderingEngine::Shutdown(GameState& rGameState)
{
	m_spriteRenderer.Clear();
	ClearFramebuffer(rGameState);
}

void RenderingEngine::RenderFrame(GameState& rGameState, Camera2D& camera)
{
	PROFILE();

	// Always render the scene once to the framebuffer
	RenderScene(rGameState, camera);

	// In fullscreen mode: blit the framebuffer to screen
	BlitFramebufferToScreen(rGameState);

	{
		PROFILE_SCOPE("SwapWindow");
		SDL_GL_SwapWindow(rGameState.window.pWindow);
	}
}

void RenderingEngine::RenderEditorFrame(GameState& rGameState, Camera2D& camera)
{
	PROFILE();

	// Setup ImGui Rendering Frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();

	// In editor mode: display the texture in ImGui
	{
		PROFILE_SCOPE("Render::ImGui Widgets");
		if(!rGameState.editor.bShowImGui) return;

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
		
		m_imguiRenderer.Render(rGameState);
		
		if(rGameState.editor.bOpenProfiler)
		{
			PROFILE_SCOPE("Profiler");
			m_profilerWindow.DrawProfilerFlameGraph(rGameState);
		}

		// Bottom panel
		if(ImGui::Begin("Console"))
		{
			PROFILE_SCOPE("Console");
			ImGui::Text("Console Output");
			ImGui::Separator();
			ImGui::Text("Application running...");
			ImGui::Text("Viewport mode: %s", rGameState.editor.bShowImGui ? "Editor" : "Fullscreen");
			ImGui::Text("Scene rendering to texture: %dx%d", rGameState.framebufferWidth, rGameState.framebufferHeight);
		}
		ImGui::End();

		// PerfPanel
#if ENC_DEBUG
		if(rGameState.editor.bOpenPerformanceMonitor)
		{
			debug::DrawFrameStats(rGameState, g_frameStats);
		}
#endif

		// Rendering
		debug::DrawRendererStats(m_spriteRenderer);

		// Central viewport window - this displays the 3D scene
		if(ImGui::Begin("Scene Viewport"))
		{
			PROFILE_SCOPE("Scene ViewPort");
			ImVec2 content_region = ImGui::GetContentRegionAvail();
			content_region.x = utils::Max(64.0f, content_region.x);
			content_region.y = utils::Max(64.0f, content_region.y);

			// Resize framebuffer if needed
			ResizeFramebuffer(rGameState, (i32)content_region.x, (i32)content_region.y);

			// Display the rendered scene texture
			ImGui::Image((void*)(intptr_t)rGameState.colorTexture,
				ImVec2((f32)rGameState.framebufferWidth, (f32)rGameState.framebufferHeight), ImVec2(0, 1), ImVec2(1, 0));

		}
		ImGui::End();

		// Demo window
		if(rGameState.editor.bShowDemoWindow)
		{
			ImGui::ShowDemoWindow(&rGameState.editor.bShowDemoWindow);
		}

		ImGui::End();
	}

	// Prepare ImGui for Rendering
	{
		PROFILE_SCOPE("Render::ImGui::PrepareForRender");
		ImGui::Render();
	}

	// Always render the scene once to the framebuffer
	RenderScene(rGameState, camera);

	{
		PROFILE_SCOPE("Render::Blit");
		// Clear the screen and render ImGui
		SDL_GetWindowSize(rGameState.window.pWindow, &rGameState.window.width, &rGameState.window.height);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, rGameState.window.width, rGameState.window.height);
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
		SDL_GL_SwapWindow(rGameState.window.pWindow);
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

void RenderingEngine::RenderScene(GameState& rGameState, Camera2D& camera)
{
	PROFILE();

	// In full screen mode, determine the proper size for the framebuffer
	if(!rGameState.editor.bShowImGui)
	{
		SDL_GetWindowSize(rGameState.window.pWindow, &rGameState.window.width, &rGameState.window.height);
		ResizeFramebuffer(rGameState, rGameState.window.width, rGameState.window.height);
	}

	// Always render to framebuffer first
	glBindFramebuffer(GL_FRAMEBUFFER, rGameState.framebuffer);
	glViewport(0, 0, rGameState.framebufferWidth, rGameState.framebufferHeight);

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 2D Rendering Setup
	glDisable(GL_DEPTH_TEST); // Disable depth testing for 2D
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_spriteRenderer.Begin(camera, (f32)rGameState.framebufferWidth, (f32)rGameState.framebufferHeight);

	// Draw background
	m_spriteRenderer.DrawSprite({ 0, 0 }, { 2000, 2000 }, 0, { 0.1f, 0.1f, 0.2f, 1.0f });

	// Draw all sprites
	for(const RenderCommand& cmd : m_renderCommands)
	{
		m_spriteRenderer.DrawSprite(cmd.position, cmd.rotation, cmd.sprite);
	}

	// Draw UI elements (these don't move with camera)
	// You'd set up a separate UI camera/projection for this

	m_spriteRenderer.End();

	// Unbind framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Reset viewport to window size for final presentation
	glViewport(0, 0, rGameState.window.width, rGameState.window.height);
}

void RenderingEngine::BlitFramebufferToScreen(GameState& rGameState)
{
	PROFILE();
	// Blit the framebuffer to the screen in fullscreen mode
	SDL_GetWindowSize(rGameState.window.pWindow, &rGameState.window.width, &rGameState.window.height);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, rGameState.framebuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	glBlitFramebuffer(
		0, 0, rGameState.framebufferWidth, rGameState.framebufferHeight,  // src rect
		0, 0, rGameState.window.width, rGameState.window.height,  // dst rect
		GL_COLOR_BUFFER_BIT, GL_LINEAR
	);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderingEngine::CreateFramebuffer(GameState& rGameState)
{
	PROFILE();
	// Delete existing framebuffer if it exists
	if(rGameState.framebuffer)
	{
		glDeleteFramebuffers(1, &rGameState.framebuffer);
		glDeleteTextures(1, &rGameState.colorTexture);
		glDeleteTextures(1, &rGameState.depthTexture);
	}

	// Create framebuffer
	glGenFramebuffers(1, &rGameState.framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, rGameState.framebuffer);

	// Create color texture
	glGenTextures(1, &rGameState.colorTexture);
	glBindTexture(GL_TEXTURE_2D, rGameState.colorTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, rGameState.framebufferWidth, rGameState.framebufferHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rGameState.colorTexture, 0);

	// Create depth texture
	glGenTextures(1, &rGameState.depthTexture);
	glBindTexture(GL_TEXTURE_2D, rGameState.depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, rGameState.framebufferWidth, rGameState.framebufferHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, rGameState.depthTexture, 0);

	// Check framebuffer completeness
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		LOG_ERROR("Framebuffer not complete!");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderingEngine::ClearFramebuffer(GameState& rGameState)
{
	if(rGameState.framebuffer)
	{
		glDeleteFramebuffers(1, &rGameState.framebuffer);
		glDeleteTextures(1, &rGameState.colorTexture);
		glDeleteTextures(1, &rGameState.depthTexture);
	}
}

void RenderingEngine::ResizeFramebuffer(GameState& rGameState, i32 width, i32 height)
{
	if(width != rGameState.framebufferWidth || height != rGameState.framebufferHeight)
	{
		rGameState.framebufferWidth = width;
		rGameState.framebufferHeight = height;
		CreateFramebuffer(rGameState);
	}
}
