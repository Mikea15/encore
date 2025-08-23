#pragma once

#include "core/core_minimal.h"

#include "components/move_component.h"
#include "components/sprite2d_component.h"
#include "debug/extension_imgui.h"
#include "editor/editor_widget.h"
#include "memory/base_pool.h"
#include "profiler/profiler.h"
#include "utils/string_factory.h"

class MemoryMonitorWidget : public EditorWidget
{
public:
	virtual void DrawMenu() override
	{
		ImGui::MenuItem("Memory Monitor", nullptr, m_pOpenPanel);
	}

	virtual void Run(GameState& rGameState) override
	{
		if(rGameState.widgets.bMemoryMonitor)
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
						ArenaStats stats = arena_get_stats(&rGameState.arenas[i]);
						totalUsageReserved += stats.totalSize;
						currentTotalUsage += stats.usedBytes;
					}

					f32 totalUsageRatio = (f32)currentTotalUsage / totalUsageReserved;

					const char* str = StringFactory::TempFormat("%.2f%% ( %.2f MB / %.2f MB )",
						totalUsageRatio,
						(f32)BYTES_TO_MB(currentTotalUsage),
						(f32)BYTES_TO_MB(totalUsageReserved));

					ImGui::UsageProgressBar(str, totalUsageRatio / 100.0f, ImVec2(0.0f, 15.0f));
					ImGui::SameLine();
					ImGui::Text("Total Usage");
				}
				if(ImGui::CollapsingHeader("Arenas", ImGuiTreeNodeFlags_DefaultOpen))
				{
					DrawMemoryStats(rGameState.arenas[AT_GLOBAL], "Global");
					DrawMemoryStats(rGameState.arenas[AT_COMPONENTS], "Components");
					DrawMemoryStats(rGameState.arenas[AT_FRAME], "Frame");
				}
				if(ImGui::CollapsingHeader("Pools", ImGuiTreeNodeFlags_DefaultOpen))
				{
					DrawPoolUsageWidget("Move Component", MoveComponent::GetPool());
					DrawPoolUsageWidget("Sprite2D Component", Sprite2DComponent::GetPool());
				}
			}
			ImGui::End();
		}
	}

private:
	void DrawMemoryStats(Arena& arena, const char* name)
	{
		ArenaStats stats = arena_get_stats(&arena);

		const char* str = StringFactory::TempFormat("%.2f%% ( %.2f MB / %.2f MB )",
			stats.usageRatio,
			(f32)BYTES_TO_MB(stats.usedBytes),
			(f32)BYTES_TO_MB(stats.totalSize));

		ImGui::UsageProgressBar(str, stats.usageRatio / 100.0f, ImVec2(0.0f, 15.0f));
		ImGui::SameLine();
		ImGui::Text("%s", name);
	}

	template<typename T>
	void DrawPoolUsageWidget(const char* pPoolName, Pool<T>* pPool, bool bShowDetails = true)
	{
		u32 capacity = pPool->GetCapacity();
		u32 activeCount = pPool->GetActiveCount();
		u32 freeCount = pPool->GetFreeCount();
		float usagePercent = pPool->GetUsagePercentage();

		const char* str = StringFactory::TempFormat("%.2f%% ( %lu / %lu )",
			usagePercent, activeCount, capacity);

		ImGui::UsageProgressBar(str, usagePercent, ImVec2(0.0f, 15.0f)); ImGui::SameLine();
		ImGui::Text("%s", pPoolName);
	}
};
