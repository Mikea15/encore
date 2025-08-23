
#include "rendering_engine.h"

#include "debug/renderer_widget.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui/backends/imgui_impl_sdl2.h"
#include "profiler/profiler.h"
#include "profiler/profiler_section.h"

RenderingEngine::RenderingEngine()
	: m_spriteRenderer()
{ }

void RenderingEngine::Init(GameState& rGameState)
{
	m_spriteRenderer.Init();

	m_renderCommands.reserve(100'000);

	CreateFramebuffer(rGameState);
}

void RenderingEngine::Shutdown(GameState& rGameState)
{
	m_spriteRenderer.Clear();
	ClearFramebuffer(rGameState);
}

void RenderingEngine::RenderFrame(GameState& rGameState, Camera2D& camera)
{
	PROFILE();

	SDL_GetWindowSize(rGameState.window.pWindow, &rGameState.window.width, &rGameState.window.height);
	ResizeFramebuffer(rGameState, rGameState.window.width, rGameState.window.height);

	// Always render the scene once to the framebuffer
	RenderScene(rGameState, camera);

	// In fullscreen mode: blit the framebuffer to screen
	BlitFramebufferToScreen(rGameState);

	{
		PROFILE_SCOPE("SwapWindow");
		SDL_GL_SwapWindow(rGameState.window.pWindow);
	}
}

// Setup ImGui Rendering Frame
void RenderingEngine::NewFrame_ImGui()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();
}

void RenderingEngine::RenderEditorFrame(GameState& rGameState, Camera2D& camera)
{
	PROFILE();

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

// Prepare ImGui for Rendering
void RenderingEngine::EndFrame_ImGui()
{
	PROFILE();
	ImGui::Render();
}

void RenderingEngine::PushRenderCommand(RenderCommand cmd)
{
	m_renderCommands.push_back(cmd);
}

void RenderingEngine::ClearRenderCommands()
{
	m_renderCommands.clear();
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

void RenderingEngine::RenderScene(GameState& rGameState, Camera2D& camera)
{
	PROFILE();

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
