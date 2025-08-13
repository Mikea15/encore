#pragma once

#include "core/core_minimal.h"

#include "extension_imgui.h"
#include "memory/base_arena.h"
#include "memory/base_pool.h"
#include "utils/string_factory.h"

namespace debug 
{
	static void DrawMemoryStats(Arena& arena, const char* name) 
	{
		ArenaStats stats = arena_get_stats(&arena);
			
		const char* str = StringFactory::TempFormat("%.1f%% ( Used: %.2f KB | Free: %.2f KB )",
			stats.usageRatio, 
			(f32)BYTES_TO_KB(stats.usedBytes), 
			(f32)BYTES_TO_KB(stats.freeBytes));

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

		const char* str = StringFactory::TempFormat("%.1f%% (Used: %lu | Free: %lu)",
			usagePercent, activeCount, freeCount);

		ImGui::UsageProgressBar(str, usagePercent, ImVec2(0.0f, 15.0f)); ImGui::SameLine();
		ImGui::Text("%s", pPoolName);
	}
}
