#pragma once
#include "core/core_minimal.h"

#include "editor/editor_widget.h"
#include "gfx/frame_stats.h"
#include "utils/string_factory.h"

class PerformanceMonitorWidget : public EditorWidget
{
public:
	virtual void Run(GameState& rGameStates) override
	{
		if (ImGui::Begin("Performance Monitor"))
		{
			// Current frame m_renderStats
			ImGui::Text("Average FPS: %.1f", g_frameStats.avgFps);
			ImGui::Text("Average FrameTime: %.3f ms", 1000.0f / g_frameStats.avgFps);

			ImGui::Separator();

			const u32 index = g_frameStats.frameCount % g_frameStats.sampleCount;

			// Frame time graph
			{
				const char* str = StringFactory::TempFormat("%.1f ms", g_frameStats.pFrameTimes[index]);
				ImGui::Text("Frame Time Graph (ms):");
				ImGui::PlotLines("##frametime", g_frameStats.pFrameTimes, g_frameStats.sampleCount, 0,
				                 str, 0.0f, 50.0f, // Max ms scale
				                 ImVec2(-1, 80));
			}
			{
				const char* str = StringFactory::TempFormat("%.1f fps", g_frameStats.pFramesPerSecond[index]);
				ImGui::Text("FPS Graph:");
				ImGui::PlotLines("##fps", g_frameStats.pFramesPerSecond, static_cast<i32>(g_frameStats.sampleCount), 0,
				                 str, 0.0f, 240.0f, // Max FPS scale
				                 ImVec2(-1, 80));
			}
		}
		ImGui::End();
	}
};
