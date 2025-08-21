#include "core/core_minimal.h"

#include "editor/editor_widget.h"
#include <gfx/texture_manager.h>
#include <utils/string_factory.h>
#include <debug/extension_imgui.h>

class GpuStatsWidget : public EditorWidget
{
public:
	virtual void Run(GameState& rGameStates) override
	{
		if(ImGui::Begin("GPU Stats"))
		{
			TextureManager::GPUMemStats stats;
			stats = TextureManager::GetInstance().GetGPUMemoryUsage();

			u64 usedBytes = stats.total - stats.available;
			const f32 fraction = (f32)usedBytes / stats.total;
			const char* str = StringFactory::TempFormat("%.1f%% ( %.1f MB / %.1f MB )",
				fraction,
				(f32)BYTES_TO_MB(usedBytes),
				(f32)BYTES_TO_MB(stats.total));

			ImGui::UsageProgressBar(str, fraction / 100.0f, ImVec2(0.0f, 15.0f));
			ImGui::SameLine();  ImGui::Text("GPU Memory Usage");
		}
		ImGui::End();
	}
};
