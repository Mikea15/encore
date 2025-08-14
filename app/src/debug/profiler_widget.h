
#pragma once

#include "core/core_minimal.h"

#include "profiler/profiler.h"
#include "imgui/imgui.h"
#include <algorithm>
#include "extension_imgui.h"
#include <set>
#include <utils/utils_time.h>

namespace debug 
{
	static void DrawProfiler()
	{
		if(ImGui::Begin("Profiler"))
		{
			static bool bShowAllThreads = true;
			static bool bGroupByThread = false;
			static std::thread::id selectedThreadId;
			static bool bHasSelectedThread = false;

			// Controls
			ImGui::Checkbox("All Threads", &bShowAllThreads);
			ImGui::SameLine();
			ImGui::Checkbox("Group by Thread", &bGroupByThread);

			// Get profiling data based on display mode
			std::vector<ProfilerEntry> entries;

			if(bShowAllThreads)
			{
				entries = Profiler::GetInstance().GetAllThreadsEntries();
			}
			else
			{
				if(bHasSelectedThread)
				{
					// If you add GetThreadEntries method to Profiler
					// entries = Profiler::GetInstance().GetThreadEntries(selectedThreadId);
					entries = Profiler::GetInstance().GetCurrentThreadEntries(); // Fallback for now
				}
				else
				{
					entries = Profiler::GetInstance().GetCurrentThreadEntries();
				}
			}

			// Thread selection dropdown (when not showing all threads)
			if(!bShowAllThreads)
			{
				ImGui::SameLine();

				// Get available threads
				std::set<std::thread::id> availableThreads;
				auto allEntries = Profiler::GetInstance().GetAllThreadsEntries();
				for(const auto& entry : allEntries)
				{
					availableThreads.insert(entry.threadId);
				}

				if(ImGui::BeginCombo("Thread", bHasSelectedThread ? Profiler::GetInstance().GetThreadName(selectedThreadId).c_str() : "Select Thread"))
				{
					for(const auto& threadId : availableThreads)
					{
						bool isSelected = (bHasSelectedThread && selectedThreadId == threadId);
						std::string threadName = Profiler::GetInstance().GetThreadName(threadId);

						if(ImGui::Selectable(threadName.c_str(), isSelected))
						{
							selectedThreadId = threadId;
							bHasSelectedThread = true;
							// Update entries for selected thread
							entries = Profiler::GetInstance().GetCurrentThreadEntries(); // Update this when GetThreadEntries is available
						}

						if(isSelected)
						{
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}
			}

			// Calculate total time and thread statistics
			u64 totalTime = 0;
			std::unordered_map<std::thread::id, u64> threadTotals;
			std::unordered_map<std::thread::id, std::vector<ProfilerEntry>> threadGroups;

			for(const auto& entry : entries)
			{
				if(entry.depth == 0)
				{
					totalTime += entry.duration;
					threadTotals[entry.threadId] += entry.duration;
				}
				threadGroups[entry.threadId].push_back(entry);
			}

			// Display frame time and thread breakdown
			ImGui::Text("FrameTime: %.3f ms", NS_TO_MS(totalTime));

			if(bShowAllThreads && threadTotals.size() > 1)
			{
				ImGui::Text("Threads: %zu", threadTotals.size());

				// Show thread time breakdown
				for(const auto& [threadId, duration] : threadTotals)
				{
					ImGui::Text("  %s: %.2f ms (%.1f%%)",
						Profiler::GetInstance().GetThreadName(threadId).c_str(),
						NS_TO_MS(duration),
						totalTime > 0 ? (float)duration / totalTime * 100.0f : 0.0f);
				}
				ImGui::Separator();
			}

			if(ImGui::BeginTable("PROFILER_TABLE", bGroupByThread && bShowAllThreads ? 3 : 2,
				ImGuiTableFlags_Sortable | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Borders))
			{
				ImGui::TableSetupScrollFreeze(0, 1);
				ImGui::TableSetupColumn("Function", ImGuiTableColumnFlags_WidthStretch);
				if(bGroupByThread && bShowAllThreads)
				{
					ImGui::TableSetupColumn("Thread", ImGuiTableColumnFlags_WidthFixed, 100.0f);
				}
				ImGui::TableSetupColumn("Duration", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableHeadersRow();

				// Sort entries by duration (descending)
				std::vector<ProfilerEntry> sortedEntries = entries;
				std::sort(sortedEntries.begin(), sortedEntries.end(),
					[](const ProfilerEntry& a, const ProfilerEntry& b) {
						return a.duration > b.duration;
					});

				if(bGroupByThread && bShowAllThreads && threadGroups.size() > 1)
				{
					// Display grouped by thread
					for(const auto& [threadId, threadEntries] : threadGroups)
					{
						// Thread header row
						ImGui::TableNextRow();
						ImGui::TableNextColumn();

						std::string threadHeader = "=== " + Profiler::GetInstance().GetThreadName(threadId) + " ===";
						ImU32 headerColor = ImGui::GetColorU32(ImGuiCol_HeaderActive);
						ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, headerColor);
						ImGui::Text("%s", threadHeader.c_str());

						ImGui::TableNextColumn();
						ImGui::Text("Thread");
						ImGui::TableNextColumn();
						ImGui::Text("%.2f ms", NS_TO_MS(threadTotals[threadId]));

						// Sort thread entries
						std::vector<ProfilerEntry> sortedThreadEntries = threadEntries;
						std::sort(sortedThreadEntries.begin(), sortedThreadEntries.end(),
							[](const ProfilerEntry& a, const ProfilerEntry& b) {
								return a.threadId > b.threadId;
							});

						// Display entries for this thread
						for(const auto& entry : sortedThreadEntries)
						{
							ImGui::TableNextRow();
							ImGui::TableNextColumn();

							// Indent based on depth
							std::string indent(entry.depth * 2, ' ');
							ImGui::Text("%s%s", indent.c_str(), entry.section.c_str());

							ImGui::TableNextColumn();
							ImGui::Text("%s", Profiler::GetInstance().GetThreadName(entry.threadId).c_str());

							ImGui::TableNextColumn();
							const char* str = StringFactory::TempFormat("%.3f ms", NS_TO_MS((f32)entry.duration));
							const float percentage = totalTime > 0 ? (float)entry.duration / totalTime : 0.0f;
							ImGui::UsageProgressBar(str, percentage, ImVec2(-1.0f, 15.0f));
						}
					}
				}
				else
				{
					// Display all entries in one list
					for(const auto& entry : sortedEntries)
					{
						ImGui::TableNextRow();
						ImGui::TableNextColumn();

						// Indent based on depth
						std::string indent(entry.depth * 2, ' ');
						ImGui::Text("%s%s", indent.c_str(), entry.section.c_str());

						if(bGroupByThread && bShowAllThreads)
						{
							ImGui::TableNextColumn();
							ImGui::Text("%s", Profiler::GetInstance().GetThreadName(entry.threadId).c_str());
						}

						ImGui::TableNextColumn();

						const char* str = StringFactory::TempFormat("%.3f ms", NS_TO_MS((f32)entry.duration));
						const float percentage = totalTime > 0 ? (float)entry.duration / totalTime : 0.0f;
						ImGui::UsageProgressBar(str, percentage, ImVec2(-1.0f, 15.0f));
					}
				}

				ImGui::EndTable();
			}

			// Additional statistics
			if(entries.size() > 0)
			{
				ImGui::Separator();
				ImGui::Text("Total Entries: %zu", entries.size());

				if(bShowAllThreads && threadTotals.size() > 1)
				{
					// Find heaviest thread
					auto heaviestThread = std::max_element(threadTotals.begin(), threadTotals.end(),
						[](const auto& a, const auto& b) { return a.second < b.second; });

					if(heaviestThread != threadTotals.end())
					{
						ImGui::Text("Heaviest Thread: %s (%.2f ms)",
							Profiler::GetInstance().GetThreadName(heaviestThread->first).c_str(),
							NS_TO_MS(heaviestThread->second));
					}
				}
			}
		}
		ImGui::End();
	}
}
