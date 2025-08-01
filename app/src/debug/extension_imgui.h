#pragma once

#include "imgui/imgui.h"

namespace ImGui 
{
	void UsageProgressBar(const char* label, float fraction, ImVec2 size = ImVec2(-1.0f, 0.0f))
	{
		// Determine color based on fraction
		ImVec4 color;
		if (fraction < 0.25f) {
			color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // Green
		}
		else if (fraction < 0.5f) {
			color = ImVec4(0.5f, 1.0f, 0.0f, 1.0f); // Yellow-green
		}
		else if (fraction < 0.75f) {
			color = ImVec4(1.0f, 0.65f, 0.0f, 1.0f); // Orange
		}
		else {
			color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // Red
		}

		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, color);
		ImGui::ProgressBar(fraction, size, label);
		ImGui::PopStyleColor();
	}
}
