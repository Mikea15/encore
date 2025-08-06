
#pragma once

#include "core/core_minimal.h"

#include "profiler/profiler.h"
#include "imgui/imgui.h"
#include <algorithm>
#include "extension_imgui.h"

namespace debug {
	void DrawProfiler()
	{
		if(ImGui::Begin("Profiler"))
		{
			u32 totalTime = 0;
			for(const Profiler::Entry& entry : g_profiler.entries)
			{
				totalTime += entry.duration;
			}
			ImGui::Text("FrameTime: %.3f ms", NsToMilliseconds(totalTime));


			if(ImGui::BeginTable("PROFILER_TABLE", 2, ImGuiTableFlags_None))
			{
				ImGui::TableSetupScrollFreeze(0, 1);
				ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);    // Left side
				ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);    // Right side
				ImGui::TableNextRow();

				std::sort(g_profiler.entries.begin(), g_profiler.entries.end(),
					[](const Profiler::Entry& a, const Profiler::Entry& b) {
						return a.duration > b.duration;
					});

				for(const Profiler::Entry& entry : g_profiler.entries)
				{
					ImGui::TableNextColumn();
					ImGui::Text("%s", entry.section.c_str());

					ImGui::TableNextColumn();

					char buff[24];
					sprintf_s(buff, "%.3f ms", NsToMilliseconds(entry.duration));
					ImGui::UsageProgressBar(buff, (float)entry.duration / totalTime, ImVec2(-1.0f, 15.0f));

					ImGui::TableNextRow();
				}
			}
			ImGui::EndTable();

			//ImDrawList* draw_list = ImGui::GetWindowDrawList();
			//ImGui::Text("Gradients");
			//ImVec2 gradient_size = ImVec2(ImGui::CalcItemWidth(), ImGui::GetFrameHeight());
			//{
			//	ImVec2 p0 = ImGui::GetCursorScreenPos();
			//	ImVec2 p1 = ImVec2(p0.x + gradient_size.x, p0.y + gradient_size.y);
			//	ImU32 col = ImGui::GetColorU32(IM_COL32(20, 30, 120, 255));
			//	draw_list->AddRectFilled(p0, p1, col);
			//	draw_list->AddText(ImVec2((p0.x + p1.x) * 0.5f, p0.y), ImGui::GetColorU32(IM_COL32_WHITE), "text");
			//	// ImGui::InvisibleButton("##gradient1", gradient_size);
			//}
		}
		ImGui::End();
	}
}
