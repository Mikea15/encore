#pragma once

#include "core/core_minimal.h"

#include "game_state.h"
#include "components/move_component.h"
#include "components/sprite2d_component.h"
#include "debug/memory_widget.h"
#include "profiler/profiler.h"
#include "profiler/profiler_section.h"

class ImGuiWidget
{
public:
	virtual void Draw(GameState& gameState) = 0;
};

class ImGuiMemoryMonitor : public ImGuiWidget
{
public: 
	virtual void Draw(GameState& gameState) override
	{
		if(gameState.editor.bOpenMemoryMonitor)
		{
			PROFILE_SCOPE("MemoryMonitor");
			if(ImGui::Begin("Profiler - Memory"))
			{
				if(ImGui::CollapsingHeader("Total", ImGuiTreeNodeFlags_DefaultOpen))
				{
					u32 totalUsageReserved = 0;
					u32 currentTotalUsage = 0;
					for(int i = 0; i < AT_COUNT; i++)
					{
						ArenaStats stats = arena_get_stats(&gameState.arenas[i]);
						totalUsageReserved += stats.totalSize;
						currentTotalUsage += stats.usedBytes;
					}

					f32 totalUsageRatio = (f32)currentTotalUsage / totalUsageReserved;

					const char* str = StringFactory::TempFormat("%.1f%% ( Used: %.2f KB )",
						totalUsageRatio,
						(f32)BYTES_TO_KB(currentTotalUsage));

					ImGui::UsageProgressBar(str, totalUsageRatio / 100.0f, ImVec2(0.0f, 15.0f));
					ImGui::SameLine();
					ImGui::Text("Total Usage");
				}
				if(ImGui::CollapsingHeader("Arenas", ImGuiTreeNodeFlags_DefaultOpen))
				{
					debug::DrawMemoryStats(gameState.arenas[AT_GLOBAL], "Global");
					debug::DrawMemoryStats(gameState.arenas[AT_COMPONENTS], "Components");
					debug::DrawMemoryStats(gameState.arenas[AT_FRAME], "Frame");
				}
				if(ImGui::CollapsingHeader("Pools", ImGuiTreeNodeFlags_DefaultOpen))
				{
					debug::DrawPoolUsageWidget("Move Component", MoveComponent::GetPool());
					debug::DrawPoolUsageWidget("Sprite2D Component", Sprite2DComponent::GetPool());
				}
			}
			ImGui::End();
		}
	}
};

class ImGuiMenu : public ImGuiWidget
{
public:
	virtual void Draw(GameState& gameState) override
	{
		if(ImGui::BeginMenuBar())
		{
			PROFILE_SCOPE("Menu");
			if(ImGui::BeginMenu("Window"))
			{
				ImGui::MenuItem("Profiler", nullptr, &gameState.editor.bOpenProfiler);
				ImGui::MenuItem("Performance Monitor", nullptr, &gameState.editor.bOpenPerformanceMonitor);
				ImGui::MenuItem("Memory Monitor", nullptr, &gameState.editor.bOpenMemoryMonitor);

				ImGui::EndMenu();
			}
			if(ImGui::BeginMenu("Help"))
			{
				ImGui::MenuItem("Toggle In-Game ImGui", nullptr, &gameState.bShowInGameImGui);
				ImGui::MenuItem("Toggle ImGui", "TAB", &gameState.editor.bShowImGui);
				ImGui::MenuItem("Demo Window", nullptr, &gameState.editor.bShowDemoWindow);

				ImGui::Text("TAB: Toggle Editor Mode");
				ImGui::Text("ESC: Exit application");
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}
	}
};

class RendererImGui
{
public:
	void RegisterWidget(ImGuiWidget* widget)
	{
		m_widgets.push_back(widget);
	}

	void Render(GameState& rGameState)
	{
		for(auto* widget : m_widgets)
		{
			widget->Draw(rGameState);
		}
	}

private:
	std::vector<ImGuiWidget*> m_widgets;
};
