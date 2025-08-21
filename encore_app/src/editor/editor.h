#pragma once

#include "core/core_minimal.h"

#include "editor_profiler.h"
#include "game_state.h"
#include "gfx/camera_2d.h"
#include "gfx/types.h"
#include "profiler/profiler_section.h"
#include "editor_widget.h"
#include "widgets/gpu_stats_widget.h"
#include "widgets/memory_monitor_widget.h"
#include "widgets/menu_widget.h"
#include <gfx/frame_stats.h>
#include <debug/framerate_widget.h>
#include <gfx/rendering_engine.h>

class Editor
{
public:
	void Init(GameState* pGameState, RenderingEngine* pRenderingEngine)
	{
		m_pGameState = pGameState;
		Assert(m_pGameState);
		m_pRenderingEngine = pRenderingEngine;
		Assert(m_pRenderingEngine);

		m_editorWidgets.push_back(new MenuWidget());
		m_editorWidgets.push_back(new MemoryMonitorWidget());
		m_editorWidgets.push_back(new GpuStatsWidget());
	}

	void Shutdown()
	{
		for(auto* pWidget : m_editorWidgets)
		{
			delete pWidget;
		}
		m_editorWidgets.clear();
	}

	void HandleInput(const SDL_Event& event)
	{
		m_cameraInput = {};

		switch(event.type)
		{
		case SDL_KEYDOWN:
			if(event.key.keysym.sym == SDLK_TAB)
			{
				m_pGameState->editor.bShowImGui = !m_pGameState->editor.bShowImGui;
			}
			break;
		default: break;
		}

		const u8* keyboardState = SDL_GetKeyboardState(nullptr);

		if(keyboardState[SDL_SCANCODE_A]) { m_cameraInput.x = 1.0f; }
		if(keyboardState[SDL_SCANCODE_D]) { m_cameraInput.x = -1.0f; }
		if(keyboardState[SDL_SCANCODE_S]) { m_cameraInput.y = 1.0f; }
		if(keyboardState[SDL_SCANCODE_W]) { m_cameraInput.y = -1.0f; }
		if(keyboardState[SDL_SCANCODE_Q]) { m_cameraInput.z = 1.0f; }
		if(keyboardState[SDL_SCANCODE_E]) { m_cameraInput.z = -1.0f; }

		if(m_cameraInput != Vec3(0.0f))
		{
			m_cameraInput = glm::normalize(m_cameraInput);
		}
	}

	void Update(float deltaTime)
	{
		PROFILE();

		const float cameraSpeed = 100.0f;

		{
			PROFILE_SCOPE("Editor Camera Update");

			m_cameraInput = glm::normalize(m_cameraInput);

			m_camera.zoom += m_cameraInput.z * deltaTime;
			m_camera.zoom = utils::Clamp(m_camera.zoom, 0.01f, 100.0f);

			m_camera.cameraVelocity = glm::mix(m_camera.cameraVelocity,
				Vec2(m_cameraInput.x, m_cameraInput.y) * cameraSpeed,
				m_camera.cameraDamping * deltaTime);
		}
	}

	void RenderEditor()
	{
		Assert(m_pGameState);

		// In editor mode: display the texture in ImGui
		{
			PROFILE_SCOPE("Render::ImGui Widgets");
			if(!m_pGameState->editor.bShowImGui) return;

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

			// Render all registered widgets
			RenderWidgets();

			if(m_pGameState->editor.bOpenProfiler)
			{
				PROFILE_SCOPE("Profiler");
				m_profilerWindow.DrawProfilerFlameGraph(*m_pGameState);
			}

			// Bottom panel
			if(ImGui::Begin("Console"))
			{
				PROFILE_SCOPE("Console");
				ImGui::Text("Console Output");
				ImGui::Separator();
				ImGui::Text("Application running...");
				ImGui::Text("Viewport mode: %s", m_pGameState->editor.bShowImGui ? "Editor" : "Fullscreen");
				ImGui::Text("Scene rendering to texture: %dx%d", m_pGameState->framebufferWidth, m_pGameState->framebufferHeight);
			}
			ImGui::End();

			// PerfPanel
#if ENC_DEBUG
			if(m_pGameState->editor.bOpenPerformanceMonitor)
			{
				debug::DrawFrameStats(*m_pGameState, g_frameStats);
			}
#endif

			// Rendering
			// debug::DrawRendererStats(m_spriteRenderer);

			// Central viewport window - this displays the 3D scene
			if(ImGui::Begin("Scene Viewport"))
			{
				PROFILE_SCOPE("Scene ViewPort");
				ImVec2 content_region = ImGui::GetContentRegionAvail();
				content_region.x = utils::Max(64.0f, content_region.x);
				content_region.y = utils::Max(64.0f, content_region.y);

				// Resize framebuffer if needed
				m_pRenderingEngine->ResizeFramebuffer(*m_pGameState, (i32)content_region.x, (i32)content_region.y);

				// Display the rendered scene texture
				ImGui::Image((void*)(intptr_t)m_pGameState->colorTexture,
					ImVec2((f32)m_pGameState->framebufferWidth, (f32)m_pGameState->framebufferHeight), ImVec2(0, 1), ImVec2(1, 0));

			}
			ImGui::End();

			// Demo window
			if(m_pGameState->editor.bShowDemoWindow)
			{
				ImGui::ShowDemoWindow(&m_pGameState->editor.bShowDemoWindow);
			}

			ImGui::End();
		}
	}

	void RenderWidgets()
	{
		for(auto* pWidget : m_editorWidgets)
		{
			pWidget->Run(*m_pGameState);
		}
	}

	Camera2D& GetCamera() { return m_camera; }

private:
	GameState* m_pGameState = nullptr;
	RenderingEngine* m_pRenderingEngine = nullptr;

	Camera2D m_camera;
	Vec3 m_cameraInput;
	ProfilerWindow m_profilerWindow;

	std::vector<EditorWidget*> m_editorWidgets;
};


