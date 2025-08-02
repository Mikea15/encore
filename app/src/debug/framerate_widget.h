#pragma once

#include "imgui/imgui.h"

#include "base_arena.h"

#include <vector>
#include <algorithm>

class FrameTimeTracker
{
public:
	void Init(Arena& arena) 
	{
		m_pFrameTimes = arena_alloc_array(&arena, f32, m_sampleCount);
		m_pFramesPerSecond = arena_alloc_array(&arena, f32, m_sampleCount);

		AssertMsg(m_pFrameTimes, "Could not allocate, check arena usage. ");
		AssertMsg(m_pFramesPerSecond, "Could not allocate, check arena usage. ");
	}

	void Update(float deltaTime) {
		float current_fps = (deltaTime > 0.0f) ? (1.0f / deltaTime) : 0.0f;

		const u32 index = m_frameCount++ % m_sampleCount;
		m_framesOneSec++;

		// Add frametime sample
		m_pFrameTimes[index] = deltaTime * 1000.0f;

		// Add FPS sample
		m_pFramesPerSecond[index] = current_fps;

		// Update statistics
		m_oneSecond += deltaTime;
		if (m_oneSecond >= 1.0f)
		{
			m_averageFramesPerSecond = static_cast<float>(m_framesOneSec);

			m_oneSecond -= 1.0f;
			m_framesOneSec = 0;
		}
	}

	void RenderImGuiWindow() {
		if (ImGui::Begin("Performance Monitor"))
		{
			// Current frame stats
			ImGui::Text("Average FPS: %.1f", m_averageFramesPerSecond);
			ImGui::Text("Average FrameTime: %.3f ms", 1000.0f / m_averageFramesPerSecond);

			ImGui::Separator();

			// Frame time graph
			ImGui::Text("Frame Time Graph (ms):");
			ImGui::PlotLines("##frametime", m_pFrameTimes, m_sampleCount,
				0, nullptr, 0.0f, 50.0f,  // Max ms scale 
				ImVec2(-1, 80));

			ImGui::Text("FPS Graph:");
			ImGui::PlotLines("##fps",
				m_pFramesPerSecond,
				m_sampleCount,
				0, nullptr, 0.0f, 240.0f,  // Max FPS scale
				ImVec2(-1, 80));
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
			ImGui::Text("Avg FPS: %.1f", m_averageFramesPerSecond);
			ImGui::Text("Avg FrameTime: %.3f ms", 1000.0f / m_averageFramesPerSecond);

			ImGui::PlotLines("##mini",
				m_pFramesPerSecond,
				m_sampleCount,
				0,
				nullptr,
				0.0f,
				50.0f,
				ImVec2(-1, 30));
		}
		ImGui::End();
	}

private:
	u32 m_sampleCount = 120;
	f32* m_pFrameTimes = nullptr;
	f32* m_pFramesPerSecond = nullptr;
	u32 m_frameCount = 0;
	f32 m_averageFramesPerSecond = 0.0f;
	f32 m_oneSecond = 0.0f;

	u32 m_framesOneSec = 0;
};
