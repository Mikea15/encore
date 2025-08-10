#pragma once

#include "core/core_minimal.h"

#include "extension_imgui.h"
#include "memory/base_arena.h"
#include "utils/string_factory.h"

namespace debug 
{
	static void DrawMemoryStats(Arena& arena, const char* name) 
	{
		if (ImGui::Begin("Memory Stats")) 
		{
			ArenaStats stats = arena_get_stats(&arena);
			
			const char* str = StringFactory::TempFormat(
				"\"%s\">> %.1f%% ( Used: %.2f KB | Free: %.2f KB )",
				name, 
				stats.usageRatio, 
				(f32)BYTES_TO_KB(stats.usedBytes), 
				(f32)BYTES_TO_KB(stats.freeBytes));

			ImGui::UsageProgressBar(str, stats.usageRatio / 100.0f, ImVec2(-1.0f, 15.0f));

		}
		ImGui::End();
	}
}
