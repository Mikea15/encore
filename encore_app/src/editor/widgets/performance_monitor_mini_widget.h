#pragma once
#include "core/core_minimal.h"

#include "editor/editor_widget.h"
#include "gfx/frame_stats.h"

class PerformanceMonitorMiniWidget : public EditorWidget
{
public:
	virtual void Run(GameState& rGameStates) override
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
			ImGui::Text("Avg FPS: %.1f", g_frameStats.avgFps);
			ImGui::Text("Avg FrameTime: %.3f ms", SEC_TO_MS(1.0f) / g_frameStats.avgFps);

			ImGui::PlotLines("##mini", g_frameStats.pFramesPerSecond, static_cast<i32>(g_frameStats.sampleCount),
			                 0, nullptr, 0.0f, 240.0f, ImVec2(-1, 30));
		}
		ImGui::End();
	}
};
