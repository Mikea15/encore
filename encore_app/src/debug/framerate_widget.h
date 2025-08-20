#pragma once

#include "core/core_minimal.h"

#include "game_state.h"
#include "imgui/imgui.h"
#include "utils/string_factory.h"

#include "gfx/frame_stats.h"

namespace debug
{
#if ENC_DEBUG
	static void DrawFrameStats(GameState& gameState, FrameStats& stats)
	{
		if (ImGui::Begin("Performance Monitor"))
		{
			// Current frame m_renderStats
			ImGui::Text("Average FPS: %.1f", stats.avgFps);
			ImGui::Text("Average FrameTime: %.3f ms", 1000.0f / stats.avgFps);

			ImGui::Separator();

			const u32 index = stats.frameCount % stats.sampleCount;

			// Frame time graph
			{
				const char* str = StringFactory::TempFormat("%.1f ms", stats.pFrameTimes[index]);
				ImGui::Text("Frame Time Graph (ms):");
				ImGui::PlotLines("##frametime", stats.pFrameTimes, stats.sampleCount, 0, 
					str, 0.0f, 50.0f, // Max ms scale
					ImVec2(-1, 80));
			}
			{
				const char* str = StringFactory::TempFormat("%.1f fps", stats.pFramesPerSecond[index]);
				ImGui::Text("FPS Graph:");
				ImGui::PlotLines("##fps", stats.pFramesPerSecond, static_cast<i32>(stats.sampleCount), 0, 
					str, 0.0f, 240.0f, // Max FPS scale
					ImVec2(-1, 80));
			}
		}
		ImGui::End();
	}

	static void DrawFrameStatsCompact(FrameStats& stats)
	{
		// Compact overlay in corner
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_NoNav;

		const float PAD = 10.0f;
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImVec2 work_pos = viewport->WorkPos;
		ImVec2 work_size = viewport->WorkSize;
		ImVec2 window_pos, window_pos_pivot;

		// Position in top-right corner
		window_pos.x = work_pos.x + work_size.x - PAD;
		window_pos.y = work_pos.y + PAD;
		window_pos_pivot.x = 1.0f;
		window_pos_pivot.y = 0.0f;

		ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
		ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background

		if (ImGui::Begin("Performance Overlay", nullptr, window_flags))
		{
			ImGui::Text("Avg FPS: %.1f", stats.avgFps);
			ImGui::Text("Avg FrameTime: %.3f ms", SEC_TO_MS(1.0f) / stats.avgFps);

			ImGui::PlotLines("##mini", stats.pFramesPerSecond, static_cast<i32>(stats.sampleCount),
			                 0, nullptr, 0.0f, 240.0f, ImVec2(-1, 30));
		}
		ImGui::End();
	}
#endif
};
