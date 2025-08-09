#pragma once

#include "core/core_minimal.h"
#include "editor_profiler.h"
#include "game_state.h"

#if ENC_DEBUG
#define ENABLE_EDITOR 1
#else
#define ENABLE_EDITOR 0
#endif

namespace editor
{
#if ENABLE_EDITOR
	class Editor
	{
	public:
		Editor(GameState& rGameState) 
			: m_rGameState(rGameState)
		{}

		void Render(Render2D& render2D)
		{
			PROFILE();

			if(!m_rGameState.editor.bShowImGui) 
				return;

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

			RenderMenu();

			if(m_rGameState.editor.bOpenProfiler)
			{
				m_profilerWindow.DrawProfilerFlameGraph(m_rGameState);
			}

			// DockSpace
			ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

			ImGui::End();

			if(1)
			{
				ImGui::Begin("Properties");
				ImGui::Text("Scene Properties");
				ImGui::Text("Framebuffer Size: %d x %d", m_rGameState.framebufferWidth, m_rGameState.framebufferHeight);
				ImGui::Text("Time: %.2f", time);
				ImGui::Separator();
				ImGui::Text("Controls:");
				ImGui::BulletText("F1: Toggle UI");
				ImGui::BulletText("ESC: Exit");
				ImGui::End();
			}

			if(m_rGameState.editor.bOpenMemoryMonitor)
			{
				debug::DrawMemoryStats(m_rGameState.globalArena, "Global");
				debug::DrawMemoryStats(m_rGameState.frame, "Frame");
				debug::DrawMemoryStats(m_rGameState.componentsArena, "Components");
				debug::DrawMemoryStats(m_rGameState.enemiesArena, "Enemies");
				debug::DrawMemoryStats(m_rGameState.uiArena, "UI");
			}


			// debug::DrawProfilerFlameGraph();

			// Bottom panel
			ImGui::Begin("Console");
			ImGui::Text("Console Output");
			ImGui::Separator();
			ImGui::Text("Application running...");
			ImGui::Text("Viewport mode: %s", m_rGameState.editor.bShowImGui ? "Editor" : "Fullscreen");
			ImGui::Text("Scene rendering to texture: %dx%d", m_rGameState.framebufferWidth, m_rGameState.framebufferHeight);
			ImGui::End();

			// PerfPanel
#if ENC_DEBUG
			if(m_rGameState.editor.bOpenPerformanceMonitor)
			{
				debug::DrawFrameStats(m_rGameState, g_frameStats);
			}
#endif
			// Rendering
			debug::DrawRendererStats(render2D);

			// Demo window
			if(m_rGameState.editor.bShowDemoWindow)
			{
				ImGui::ShowDemoWindow(&m_rGameState.editor.bShowDemoWindow);
			}
		}

	private:
		void RenderMenu()
		{
			if(ImGui::BeginMenuBar())
			{
				if(ImGui::BeginMenu("File"))
				{

					ImGui::EndMenu();
				}

				if(ImGui::BeginMenu("..."))
				{

					ImGui::EndMenu();
				}

				if(ImGui::BeginMenu("Window"))
				{
					ImGui::MenuItem("Memory Monitor", nullptr, m_rGameState.editor.bOpenMemoryMonitor);
					ImGui::MenuItem("Performance Monitor", nullptr, m_rGameState.editor.bOpenPerformanceMonitor);
					ImGui::MenuItem("Profiler", nullptr, m_rGameState.editor.bOpenProfiler);

					ImGui::EndMenu();
				}
				if(ImGui::BeginMenu("Help"))
				{
					ImGui::MenuItem("Toggle ImGui", "TAB", m_rGameState.editor.bShowImGui);
					ImGui::MenuItem("Demo Window", nullptr, m_rGameState.editor.bShowDemoWindow);

					ImGui::Text("TAB: Toggle Editor Mode");
					ImGui::Text("ESC: Exit application");

					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}
		}

	private:
		GameState& m_rGameState;

		ProfilerWindow m_profilerWindow;
	};
#endif
}
