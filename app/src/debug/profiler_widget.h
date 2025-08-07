
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
			u64 totalTime = 0;
			for(const Profiler::Entry& entry : g_profiler.lastEntries)
			{
				if(entry.depth == 0)
					totalTime += entry.duration;
			}
			ImGui::Text("FrameTime: %.3f ms", NsToMilliseconds(totalTime));

			if(ImGui::BeginTable("PROFILER_TABLE", 2, ImGuiTableFlags_None))
			{
				ImGui::TableSetupScrollFreeze(0, 1);
				ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);    // Left side
				ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);    // Right side
				ImGui::TableNextRow();

				std::sort(g_profiler.lastEntries.begin(), g_profiler.lastEntries.end(),
					[](const Profiler::Entry& a, const Profiler::Entry& b) {
						return a.duration > b.duration;
					});

				for(const Profiler::Entry& entry : g_profiler.lastEntries)
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
		}
		ImGui::End();
	}
}
