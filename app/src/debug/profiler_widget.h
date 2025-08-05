
#pragma once

#include "core/core_minimal.h"

#include "profiler/profiler.h"
#include "imgui/imgui.h"
#include <algorithm>
#include "extension_imgui.h"

namespace debug 
{
	void DrawProfiler()
	{
		if(ImGui::Begin("Profiler"))
		{
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
					sprintf_s(buff, "%.4f ms", (float)entry.duration / 1000.0f);
					ImGui::UsageProgressBar(buff, entry.duration / 3000.f);

					ImGui::TableNextRow();
				}
			}
			ImGui::EndTable();
		}
		ImGui::End();
	}
}
