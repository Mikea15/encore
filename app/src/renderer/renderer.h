#pragma once

#include "core/core_minimal.h"

#include "game_state.h"
#include "renderer_sprite.h"
#include "debug/profiler_widget.h"
#include "debug/memory_widget.h"
#include "debug/renderer_widget.h"

class Renderer
{
public:
	void CreateFramebuffer(GameState& gameState)
	{
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

	void ResizeFramebuffer(GameState& gameState, i32 width, i32 height)
	{
		if(width != gameState.framebufferWidth || height != gameState.framebufferHeight)
		{
			gameState.framebufferWidth = width;
			gameState.framebufferHeight = height;
			CreateFramebuffer(gameState);
		}
	}

	void RenderScene(GameState& gameState, Render2D& render2D)
	{
		// In full screen mode, determine the proper size for the framebuffer
		if(!gameState.bShowImgui)
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

		render2D.renderer.Begin(render2D.camera, (f32)gameState.framebufferWidth, (f32)gameState.framebufferHeight);

		// Draw background
		render2D.renderer.DrawSprite({ 0, 0 }, { 2000, 2000 }, 0, { 0.1f, 0.1f, 0.2f, 1.0f });

		// Draw all sprites
		for(const auto& sprite : render2D.sprites)
		{
			render2D.renderer.DrawSprite(sprite);
		}

		// Draw UI elements (these don't move with camera)
		// You'd set up a separate UI camera/projection for this

		render2D.renderer.End();

		// Unbind framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Reset viewport to window size for final presentation
		glViewport(0, 0, gameState.window.width, gameState.window.height);
	}

	void RenderImGui(GameState& gameState, Render2D& renderer)
	{
		if(!gameState.bShowImgui) return;

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
		if(ImGui::BeginMenuBar())
		{
			if(ImGui::BeginMenu("View"))
			{
				ImGui::MenuItem("Toggle ImGui", "TAB", &gameState.bShowImgui);
				ImGui::MenuItem("Demo Window", nullptr, &gameState.bShowDemoWindow);
				ImGui::EndMenu();
			}
			if(ImGui::BeginMenu("Help"))
			{
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
		ImGui::Text("Framebuffer Size: %d x %d", gameState.framebufferWidth, gameState.framebufferHeight);
		ImGui::Text("Time: %.2f", time);
		ImGui::Separator();
		ImGui::Text("Controls:");
		ImGui::BulletText("F1: Toggle UI");
		ImGui::BulletText("ESC: Exit");
		if(ImGui::Button("Reset Time"))
		{
			gameState.time = 0.0f;
		}
		ImGui::End();

		// Right panel
		debug::DrawMemoryStats(gameState.globalArena, "Global");
		debug::DrawMemoryStats(gameState.componentsArena, "Components");
		debug::DrawMemoryStats(gameState.enemiesArena, "Enemies");
		debug::DrawMemoryStats(gameState.uiArena, "UI");

		debug::DrawProfiler();

		// Bottom panel
		ImGui::Begin("Console");
		ImGui::Text("Console Output");
		ImGui::Separator();
		ImGui::Text("Application running...");
		ImGui::Text("Viewport mode: %s", gameState.bShowImgui ? "Editor" : "Fullscreen");
		ImGui::Text("Scene rendering to texture: %dx%d", gameState.framebufferWidth, gameState.framebufferHeight);
		ImGui::End();

		// PerfPanel
#if ENC_DEBUG
		gameState.track.RenderImGuiWindow();
#endif

		// Rendering
		debug::DrawRendererStats(renderer);

		// Central viewport window - this displays the 3D scene
		ImGui::Begin("Scene Viewport");

		// Get the content region size
		ImVec2 content_region = ImGui::GetContentRegionAvail();
		i32 new_width = (i32)content_region.x;
		i32 new_height = (i32)content_region.y;

		// Ensure minimum size
		if(new_width < 64) new_width = 64;
		if(new_height < 64) new_height = 64;

		// Resize framebuffer if needed
		ResizeFramebuffer(gameState, new_width, new_height);

		// Display the rendered scene texture
		ImGui::Image((void*)(intptr_t)gameState.colorTexture,
			ImVec2((f32)gameState.framebufferWidth, (f32)gameState.framebufferHeight), ImVec2(0, 1), ImVec2(1, 0));

		ImGui::End();

		// Demo window
		if(gameState.bShowDemoWindow)
		{
			ImGui::ShowDemoWindow(&gameState.bShowDemoWindow);
		}
	}

	void BlitFramebufferToScreen(GameState& gameState)
	{
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
};
