#pragma once

#include "core/core_minimal.h"

#include "imgui/imgui.h"
#include "memory/base_arena.h"

#include <vector>
#include <algorithm>

class FrameTimeTracker
{
public:
	void Init(Arena& arena) 
	{
		pFrameTimes = arena_alloc_array(&arena, f32, sampleCount);
		pFramesPerSecond = arena_alloc_array(&arena, f32, sampleCount);

		AssertMsg(pFrameTimes, "Could not allocate, check arena usage. ");
		AssertMsg(pFramesPerSecond, "Could not allocate, check arena usage. ");
	}

	u32 GetCurrentFrameIndex() const { return frameCount % sampleCount; }

	void Update(float deltaTime) {
		float current_fps = (deltaTime > 0.0f) ? (1.0f / deltaTime) : 0.0f;
		
		frameCount++;
		framesOneSec++;

		const u32 index = GetCurrentFrameIndex();

		// Add frametime sample
		pFrameTimes[index] = deltaTime * 1000.0f;

		// Add FPS sample
		pFramesPerSecond[index] = current_fps;

		// Update statistics
		oneSecond += deltaTime;
		if (oneSecond >= 1.0f)
		{
			averageFramesPerSecond = static_cast<float>(framesOneSec);

			oneSecond -= 1.0f;
			framesOneSec = 0;
		}
	}

	void RenderImGuiWindow() {
		if (ImGui::Begin("Performance Monitor"))
		{
			// Current frame stats
			ImGui::Text("Average FPS: %.1f", averageFramesPerSecond);
			ImGui::Text("Average FrameTime: %.3f ms", 1000.0f / averageFramesPerSecond);

			ImGui::Separator();

			const u32 index = GetCurrentFrameIndex();

			// Frame time graph
			{
				char buff[20];
				sprintf_s(buff, "%.1f ms", pFrameTimes[index]);

				ImGui::Text("Frame Time Graph (ms):");
				ImGui::PlotLines("##frametime", pFrameTimes, sampleCount,
					0, buff, 0.0f, 50.0f,  // Max ms scale 
					ImVec2(-1, 80));
			}
			{
				char buff[20];
				sprintf_s(buff, "%.1f fps", pFramesPerSecond[index]);

				ImGui::Text("FPS Graph:");
				ImGui::PlotLines("##fps",
					pFramesPerSecond,
					sampleCount,
					0, buff, 0.0f, 240.0f,  // Max FPS scale
					ImVec2(-1, 80));
			}
		}
		ImGui::End();
	}

	void RenderCompactOverlay() {
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

		if (ImGui::Begin("Performance Overlay", nullptr, window_flags)) {
			ImGui::Text("Avg FPS: %.1f", averageFramesPerSecond);
			ImGui::Text("Avg FrameTime: %.3f ms", 1000.0f / averageFramesPerSecond);

			ImGui::PlotLines("##mini",
				pFramesPerSecond,
				sampleCount,
				0,
				nullptr,
				0.0f,
				50.0f,
				ImVec2(-1, 30));
		}
		ImGui::End();
	}

private:
	f32* pFrameTimes = nullptr;
	f32* pFramesPerSecond = nullptr;
	f32 averageFramesPerSecond = 0.0f;
	f32 oneSecond = 0.0f;
	u32 sampleCount = 120;
	u32 frameCount = 0;
	u32 framesOneSec = 0;
};
