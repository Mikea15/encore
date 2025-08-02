#pragma once

#include "imgui/imgui.h"
#include "extension_imgui.h"
#include "core_minimal.h"
#include "base_arena.h"

namespace debug 
{
	void DrawMemoryStats(Arena& arena, const char* name) 
	{
		if (ImGui::Begin("Memory Stats")) 
		{
			ArenaStats stats = arena_get_stats(&arena);
			
			// AutoClear Arena, since Stack will be free at end of scope.
			u8 buffer[120];
			Arena scratch = arena_init(buffer, sizeof(buffer));
			const char* usageStr = arena_sprintf(&scratch, "\"%s\" >> %.1f%% ( Used: %.2f KB | Free: %.2f KB )", 
				name,
				stats.utilization,
				(f32) stats.used_bytes / (1024.f),
				(f32) stats.free_bytes / (1024.f)
			);

			ImGui::UsageProgressBar(usageStr, stats.utilization / 100.0f, ImVec2(-1.0f, 15.0f));

		}
		ImGui::End();
	}
}
