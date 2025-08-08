
#pragma once

#include "core/core_minimal.h"

#include "profiler/profiler.h"
#include "imgui/imgui.h"
#include <algorithm>
#include "extension_imgui.h"
#include <set>

namespace debug 
{
	void DrawProfiler()
	{
		if(ImGui::Begin("Profiler"))
		{
			static bool showAllThreads = true;
			static bool groupByThread = false;
			static std::thread::id selectedThreadId;
			static bool hasSelectedThread = false;

			// Controls
			ImGui::Checkbox("All Threads", &showAllThreads);
			ImGui::SameLine();
			ImGui::Checkbox("Group by Thread", &groupByThread);

			// Get profiling data based on display mode
			std::vector<ThreadSafeProfiler::Entry> entries;

			if(showAllThreads)
			{
				entries = g_profiler.GetAllThreadsEntries();
			}
			else
			{
				if(hasSelectedThread)
				{
					// If you add GetThreadEntries method to ThreadSafeProfiler
					// entries = g_profiler.GetThreadEntries(selectedThreadId);
					entries = g_profiler.GetCurrentThreadEntries(); // Fallback for now
				}
				else
				{
					entries = g_profiler.GetCurrentThreadEntries();
				}
			}

			// Thread selection dropdown (when not showing all threads)
			if(!showAllThreads)
			{
				ImGui::SameLine();

				// Get available threads
				std::set<std::thread::id> availableThreads;
				auto allEntries = g_profiler.GetAllThreadsEntries();
				for(const auto& entry : allEntries)
				{
					availableThreads.insert(entry.threadId);
				}

				if(ImGui::BeginCombo("Thread", hasSelectedThread ? g_profiler.GetThreadName(selectedThreadId).c_str() : "Select Thread"))
				{
					for(const auto& threadId : availableThreads)
					{
						bool isSelected = (hasSelectedThread && selectedThreadId == threadId);
						std::string threadName = g_profiler.GetThreadName(threadId);

						if(ImGui::Selectable(threadName.c_str(), isSelected))
						{
							selectedThreadId = threadId;
							hasSelectedThread = true;
							// Update entries for selected thread
							entries = g_profiler.GetCurrentThreadEntries(); // Update this when GetThreadEntries is available
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
			std::unordered_map<std::thread::id, std::vector<ThreadSafeProfiler::Entry>> threadGroups;

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
			ImGui::Text("FrameTime: %.3f ms", NsToMilliseconds(totalTime));

			if(showAllThreads && threadTotals.size() > 1)
			{
				ImGui::Text("Threads: %zu", threadTotals.size());

				// Show thread time breakdown
				for(const auto& [threadId, duration] : threadTotals)
				{
					ImGui::Text("  %s: %.2f ms (%.1f%%)",
						g_profiler.GetThreadName(threadId).c_str(),
						NsToMilliseconds(duration),
						totalTime > 0 ? (float)duration / totalTime * 100.0f : 0.0f);
				}
				ImGui::Separator();
			}

			if(ImGui::BeginTable("PROFILER_TABLE", groupByThread && showAllThreads ? 3 : 2,
				ImGuiTableFlags_Sortable | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Borders))
			{
				ImGui::TableSetupScrollFreeze(0, 1);
				ImGui::TableSetupColumn("Function", ImGuiTableColumnFlags_WidthStretch);
				if(groupByThread && showAllThreads)
				{
					ImGui::TableSetupColumn("Thread", ImGuiTableColumnFlags_WidthFixed, 100.0f);
				}
				ImGui::TableSetupColumn("Duration", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableHeadersRow();

				// Sort entries by duration (descending)
				std::vector<ThreadSafeProfiler::Entry> sortedEntries = entries;
				std::sort(sortedEntries.begin(), sortedEntries.end(),
					[](const ThreadSafeProfiler::Entry& a, const ThreadSafeProfiler::Entry& b) {
						return a.duration > b.duration;
					});

				if(groupByThread && showAllThreads && threadGroups.size() > 1)
				{
					// Display grouped by thread
					for(const auto& [threadId, threadEntries] : threadGroups)
					{
						// Thread header row
						ImGui::TableNextRow();
						ImGui::TableNextColumn();

						std::string threadHeader = "=== " + g_profiler.GetThreadName(threadId) + " ===";
						ImU32 headerColor = ImGui::GetColorU32(ImGuiCol_HeaderActive);
						ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, headerColor);
						ImGui::Text("%s", threadHeader.c_str());

						ImGui::TableNextColumn();
						ImGui::Text("Thread");
						ImGui::TableNextColumn();
						ImGui::Text("%.2f ms", NsToMilliseconds(threadTotals[threadId]));

						// Sort thread entries
						std::vector<ThreadSafeProfiler::Entry> sortedThreadEntries = threadEntries;
						std::sort(sortedThreadEntries.begin(), sortedThreadEntries.end(),
							[](const ThreadSafeProfiler::Entry& a, const ThreadSafeProfiler::Entry& b) {
								return a.duration > b.duration;
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
							ImGui::Text("%s", g_profiler.GetThreadName(entry.threadId).c_str());

							ImGui::TableNextColumn();
							char buff[32];
							sprintf_s(buff, "%.3f ms", NsToMilliseconds((f32)entry.duration));

							float percentage = totalTime > 0 ? (float)entry.duration / totalTime : 0.0f;
							ImGui::UsageProgressBar(buff, percentage, ImVec2(-1.0f, 15.0f));
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

						if(groupByThread && showAllThreads)
						{
							ImGui::TableNextColumn();
							ImGui::Text("%s", g_profiler.GetThreadName(entry.threadId).c_str());
						}

						ImGui::TableNextColumn();
						char buff[32];
						sprintf_s(buff, "%.3f ms", NsToMilliseconds((f32)entry.duration));

						float percentage = totalTime > 0 ? (float)entry.duration / totalTime : 0.0f;
						ImGui::UsageProgressBar(buff, percentage, ImVec2(-1.0f, 15.0f));
					}
				}

				ImGui::EndTable();
			}

			// Additional statistics
			if(entries.size() > 0)
			{
				ImGui::Separator();
				ImGui::Text("Total Entries: %zu", entries.size());

				if(showAllThreads && threadTotals.size() > 1)
				{
					// Find heaviest thread
					auto heaviestThread = std::max_element(threadTotals.begin(), threadTotals.end(),
						[](const auto& a, const auto& b) { return a.second < b.second; });

					if(heaviestThread != threadTotals.end())
					{
						ImGui::Text("Heaviest Thread: %s (%.2f ms)",
							g_profiler.GetThreadName(heaviestThread->first).c_str(),
							NsToMilliseconds(heaviestThread->second));
					}
				}
			}
		}
		ImGui::End();
	}

	static std::string formatDuration(u64 duration, bool showMicroseconds)
	{
		if(showMicroseconds)
		{
			if(duration >= 1000000) // >= 1 second
				return std::to_string(duration / 1000000.0f) + " s";
			else if(duration >= 1000) // >= 1 ms
				return std::to_string(duration / 1000.0f) + " ms";
			else
				return std::to_string(duration) + " us";
		}
		else
		{
			return std::to_string(duration / 1000.0f) + " ms";
		}
	}

	// Get color based on duration (red for slow, green for fast)
	static ImU32 getDurationColor(u64 duration, u64 totalDuration)
	{
		if(totalDuration == 0) 
			return IM_COL32(255, 255, 255, 255);

		float ratio = (float)duration / totalDuration;
		if(ratio > 0.1f) // > 10% of total time - red
			return IM_COL32(255, 100, 100, 255);
		else if(ratio > 0.05f) // > 5% - yellow
			return IM_COL32(255, 255, 100, 255);
		else // < 5% - white
			return IM_COL32(255, 255, 255, 255);
	}

	// Helper function to draw individual profile entries
	static void DrawProfileEntry(ImDrawList* draw_list, const ThreadSafeProfiler::Entry& entry,
		ImVec2 canvas_p0, ImVec2 canvas_sz, float leftMargin, float baseY, float barHeight, float barSpacing,
		u64 minTimestamp, u64 timeRange, float virtualCanvasWidth, float panOffsetPixels,
		u64 visibleStartTime, u64 visibleEndTime, u64 totalDuration, ImVec2 mousePos, bool showTooltips,
		const ThreadSafeProfiler::Entry*& hoveredEntry)
	{
		// Skip entries outside visible time range
		u64 entryStart = entry.timestamp;
		u64 entryEnd = entry.timestamp + entry.duration;

		if(entryEnd < visibleStartTime || entryStart > visibleEndTime)
			return;

		// Calculate rectangle position and size in virtual space
		if(timeRange == 0) return;

		float virtualStartX = ((float)(entryStart - minTimestamp) / (float)timeRange) * virtualCanvasWidth;
		float virtualEndX = ((float)(entryEnd - minTimestamp) / (float)timeRange) * virtualCanvasWidth;

		// Apply pan offset and convert to screen space
		float startX = canvas_p0.x + leftMargin + virtualStartX - panOffsetPixels;
		float endX = canvas_p0.x + leftMargin + virtualEndX - panOffsetPixels;

		float y = baseY + entry.depth * (barHeight + barSpacing);

		// Clamp to visible area
		startX = std::max(startX, canvas_p0.x + leftMargin);
		endX = std::min(endX, canvas_p0.x + canvas_sz.x - leftMargin);

		if(endX <= startX) return; // Skip if too narrow

		ImVec2 rectMin(startX, y);
		ImVec2 rectMax(endX, y + barHeight);

		// Color based on depth and thread
		ImU32 color = ImGui::GetPastelColor(entry.depth + std::hash<std::thread::id>{}(entry.threadId), 80);

		// Draw rectangle
		draw_list->AddRectFilled(rectMin, rectMax, color);
		draw_list->AddRect(rectMin, rectMax, ImGui::WithAlpha(color, 220), 0.0f, ImDrawFlags_None, 0.1f);

		// Add text if rectangle is wide enough
		float rectWidth = endX - startX;
		if(rectWidth > 50.0f)
		{
			std::string text = entry.section;
			ImVec2 textSize = ImGui::CalcTextSize(text.c_str());

			// Truncate text if needed
			if(textSize.x > rectWidth - 4.0f)
			{
				text = text.substr(0, (size_t)((rectWidth - 4.0f) / textSize.x * text.length())) + "...";
			}

			ImVec2 textPos(startX + 2.0f, y + (barHeight - textSize.y) * 0.5f);
			draw_list->AddText(textPos, IM_COL32(255, 255, 255, 255), text.c_str());
		}

		// Check for mouse hover
		if(showTooltips &&
			mousePos.x >= rectMin.x && mousePos.x <= rectMax.x &&
			mousePos.y >= rectMin.y && mousePos.y <= rectMax.y)
		{
			hoveredEntry = &entry;
		}
	}

	static void DrawProfilerFlameGraph()
	{
		static bool showTooltips = true;
		static float zoomLevel = 1.0f;
		static float scrollOffset = 0.0f;
		static bool isPaused = false;
		static std::vector<ThreadSafeProfiler::Entry> frozenEntries; // Store paused data
		static bool showAllThreads = true;
		static std::thread::id selectedThreadId;
		static bool hasSelectedThread = false;

		if(!ImGui::Begin("Flame Graph"))
		{
			ImGui::End();
			return;
		}

		// Get current data based on display mode
		std::vector<ThreadSafeProfiler::Entry> currentEntries;

		if(isPaused)
		{
			currentEntries = frozenEntries;
		}
		else if(showAllThreads)
		{
			currentEntries = g_profiler.GetAllThreadsEntries();
		}
		else if(hasSelectedThread)
		{
			// Get entries for specific thread - you might need to add this method to ThreadSafeProfiler
			currentEntries = g_profiler.GetCurrentThreadEntries(); // Fallback to current thread
		}
		else
		{
			currentEntries = g_profiler.GetCurrentThreadEntries();
		}

		// If we're transitioning to paused, capture the current frame
		if(isPaused && frozenEntries.empty())
		{
			frozenEntries = showAllThreads ? g_profiler.GetAllThreadsEntries() : g_profiler.GetCurrentThreadEntries();
		}

		// If unpausing, clear frozen data
		if(!isPaused && !frozenEntries.empty())
		{
			frozenEntries.clear();
		}

		// Calculate total duration and find min/max timestamps
		u64 totalDuration = 0;
		u64 minTimestamp = UINT64_MAX;
		u64 maxTimestamp = 0;
		u32 maxDepth = 0;

		// Group entries by thread for better analysis
		std::unordered_map<std::thread::id, std::vector<ThreadSafeProfiler::Entry>> threadGroups;
		std::unordered_map<std::thread::id, u64> threadDurations;

		for(const auto& entry : currentEntries)
		{
			threadGroups[entry.threadId].push_back(entry);

			if(entry.depth == 0)
			{
				threadDurations[entry.threadId] += entry.duration;
				totalDuration += entry.duration;
			}

			minTimestamp = std::min(minTimestamp, entry.timestamp);
			maxTimestamp = std::max(maxTimestamp, entry.timestamp + entry.duration);
			maxDepth = std::max(maxDepth, entry.depth);
		}

		u64 timeRange = maxTimestamp - minTimestamp;

		// Controls
		if(ImGui::Button(isPaused ? "Resume" : "Pause"))
		{
			isPaused = !isPaused;
			if(isPaused)
			{
				frozenEntries = showAllThreads ? g_profiler.GetAllThreadsEntries() : g_profiler.GetCurrentThreadEntries();
			}
		}
		ImGui::SameLine();
		ImGui::Checkbox("Show Tooltips", &showTooltips);
		ImGui::SameLine();
		ImGui::Checkbox("All Threads", &showAllThreads);

		// Thread selection dropdown
		if(!showAllThreads)
		{
			ImGui::SameLine();
			if(ImGui::BeginCombo("Thread", hasSelectedThread ? g_profiler.GetThreadName(selectedThreadId).c_str() : "Select Thread"))
			{
				for(const auto& [threadId, entries] : threadGroups)
				{
					bool isSelected = (hasSelectedThread && selectedThreadId == threadId);
					std::string threadName = g_profiler.GetThreadName(threadId);

					if(ImGui::Selectable(threadName.c_str(), isSelected))
					{
						selectedThreadId = threadId;
						hasSelectedThread = true;
					}

					if(isSelected)
					{
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}
		}

		// Display thread statistics
		if(showAllThreads && threadGroups.size() > 1)
		{
			ImGui::Text("Threads: %zu", threadGroups.size());
			ImGui::SameLine();

			// Show thread breakdown
			std::string threadBreakdown = "";
			for(const auto& [threadId, duration] : threadDurations)
			{
				if(!threadBreakdown.empty()) threadBreakdown += ", ";
				threadBreakdown += g_profiler.GetThreadName(threadId) + ": " +
					std::to_string(static_cast<int>(NsToMilliseconds(duration))) + "ms";
			}
			ImGui::TextWrapped("%s", threadBreakdown.c_str());
		}

		ImGui::Text("Frame Time: %.3f ms %s", NsToMilliseconds((f32)totalDuration), isPaused ? "(PAUSED)" : "");

		ImGui::Separator();

		// Get drawing area
		ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
		ImVec2 canvas_sz = ImGui::GetContentRegionAvail();

		// Drawing parameters
		float barHeight = 20.0f;
		float barSpacing = 2.0f;
		float leftMargin = 10.0f;
		float topMargin = 30.0f; // Space for header line
		float threadSeparatorHeight = 15.0f; // Space between threads

		// Calculate height needed for multi-thread display
		float graphHeight;
		if(showAllThreads && threadGroups.size() > 1)
		{
			// Calculate height per thread group
			float totalHeight = 0;
			for(const auto& [threadId, entries] : threadGroups)
			{
				u32 threadMaxDepth = 0;
				for(const auto& entry : entries)
				{
					threadMaxDepth = std::max(threadMaxDepth, entry.depth);
				}
				totalHeight += (threadMaxDepth + 1) * (barHeight + barSpacing) + threadSeparatorHeight;
			}
			graphHeight = totalHeight + topMargin + 50.0f;
		}
		else
		{
			graphHeight = (maxDepth + 1) * (barHeight + barSpacing) + topMargin + 50.0f;
		}

		canvas_sz.y = std::min(canvas_sz.y, graphHeight);
		ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

		// Draw background
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImU32 bgColor = ImGui::WithAlpha(ImGui::PASTEL_LIGHT_BLUE, 20);
		draw_list->AddRectFilled(canvas_p0, canvas_p1, bgColor);

		// Calculate virtual canvas size and visible viewport
		float virtualCanvasWidth = (canvas_sz.x - leftMargin * 2) * zoomLevel;
		float viewportWidth = canvas_sz.x - leftMargin * 2;

		// Pan offset in pixels
		float maxPanOffset = std::max(0.0f, virtualCanvasWidth - viewportWidth);
		float panOffsetPixels = scrollOffset * maxPanOffset;

		// Convert back to time domain
		float pixelsPerNanosecond = timeRange > 0 ? virtualCanvasWidth / (float)timeRange : 0.0f;
		u64 panOffsetTime = pixelsPerNanosecond > 0 ? (u64)(panOffsetPixels / pixelsPerNanosecond) : 0;

		u64 visibleStartTime = minTimestamp + panOffsetTime;
		u64 visibleTimeRange = zoomLevel > 0 ? (u64)((float)timeRange / zoomLevel) : timeRange;
		u64 visibleEndTime = visibleStartTime + visibleTimeRange;

		// Frame time header line
		float headerY = canvas_p0.y + 5;
		float lineY = canvas_p0.y + 20;

		// Calculate FPS
		float frameTimeMs = NsToMilliseconds(totalDuration);
		float fps = frameTimeMs > 0 ? 1000.0f / frameTimeMs : 0.0f;

		// Create the header text
		char headerText[128];
		sprintf_s(headerText, "frametime: %.2fms - %.0ffps", frameTimeMs, fps);

		// Draw the header line with text
		float headerTextWidth = ImGui::CalcTextSize(headerText).x;
		float availableWidth = canvas_sz.x - leftMargin * 2;
		float lineLength = (availableWidth - headerTextWidth - 20) / 2;

		// Left line
		draw_list->AddLine(
			ImVec2(canvas_p0.x + leftMargin, lineY - 7),
			ImVec2(canvas_p0.x + leftMargin + lineLength, lineY - 7),
			ImGui::WithAlpha(ImGui::PASTEL_LEMON_CHIFFON, 80), 1.0f
		);

		// Text in the middle
		float textX = canvas_p0.x + leftMargin + lineLength + 10;
		draw_list->AddText(ImVec2(textX, headerY), ImGui::WithAlpha(ImGui::PASTEL_LEMON_CHIFFON, 80), headerText);

		// Right line
		draw_list->AddLine(
			ImVec2(textX + headerTextWidth + 10, lineY - 7),
			ImVec2(canvas_p0.x + canvas_sz.x - leftMargin, lineY - 7),
			ImGui::WithAlpha(ImGui::PASTEL_LEMON_CHIFFON, 80), 1.0f
		);

		// Handle mouse interactions for panning and zooming
		ImVec2 mousePos = ImGui::GetMousePos();
		const ThreadSafeProfiler::Entry* hoveredEntry = nullptr;
		bool isMouseOverCanvas = mousePos.x >= canvas_p0.x && mousePos.x <= canvas_p1.x &&
			mousePos.y >= canvas_p0.y && mousePos.y <= canvas_p1.y;

		static bool isDragging = false;
		static ImVec2 dragStartPos;
		static float dragStartScrollOffset;

		if(isMouseOverCanvas)
		{
			// Handle mouse wheel for zooming
			float wheel = ImGui::GetIO().MouseWheel;
			if(wheel != 0.0f)
			{
				zoomLevel *= (1.0f + wheel * 0.1f);
				zoomLevel = std::clamp(zoomLevel, 0.1f, 10.0f);
			}

			// Handle left click dragging for panning
			if(ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				isDragging = true;
				dragStartPos = mousePos;
				dragStartScrollOffset = scrollOffset;
			}

			// Handle right click to reset zoom and pan
			if(ImGui::IsMouseClicked(ImGuiMouseButton_Right))
			{
				zoomLevel = 1.0f;
				scrollOffset = 0.0f;
			}
		}

		// Update panning while dragging
		if(isDragging)
		{
			if(ImGui::IsMouseDown(ImGuiMouseButton_Left))
			{
				ImVec2 currentMousePos = ImGui::GetMousePos();
				float dragDelta = currentMousePos.x - dragStartPos.x;

				float maxPanOffset = std::max(0.0f, virtualCanvasWidth - viewportWidth);
				if(maxPanOffset > 0.0f)
				{
					float scrollDelta = -dragDelta / maxPanOffset;
					scrollOffset = std::clamp(dragStartScrollOffset + scrollDelta, 0.0f, 1.0f);
				}
			}
			else
			{
				isDragging = false;
			}
		}

		// Draw entries grouped by thread
		float currentY = canvas_p0.y + topMargin;

		if(showAllThreads && threadGroups.size() > 1)
		{
			// Draw each thread group separately
			for(const auto& [threadId, entries] : threadGroups)
			{
				// Draw thread label
				std::string threadLabel = g_profiler.GetThreadName(threadId);
				ImU32 threadLabelColor = ImGui::WithAlpha(ImGui::PASTEL_LEMON_CHIFFON, 160);
				draw_list->AddText(ImVec2(canvas_p0.x + leftMargin, currentY), threadLabelColor, threadLabel.c_str());
				currentY += 20.0f;

				// Draw entries for this thread
				for(const auto& entry : entries)
				{
					DrawProfileEntry(draw_list, entry, canvas_p0, canvas_sz, leftMargin, currentY, barHeight, barSpacing,
						minTimestamp, timeRange, virtualCanvasWidth, panOffsetPixels, visibleStartTime, visibleEndTime,
						totalDuration, mousePos, showTooltips, hoveredEntry);
				}

				// Find max depth for this thread to advance Y properly
				u32 threadMaxDepth = 0;
				for(const auto& entry : entries)
				{
					threadMaxDepth = std::max(threadMaxDepth, entry.depth);
				}

				currentY += (threadMaxDepth + 1) * (barHeight + barSpacing) + threadSeparatorHeight;
			}
		}
		else
		{
			// Draw all entries (single thread or filtered)
			for(const auto& entry : currentEntries)
			{
				DrawProfileEntry(draw_list, entry, canvas_p0, canvas_sz, leftMargin, currentY, barHeight, barSpacing,
					minTimestamp, timeRange, virtualCanvasWidth, panOffsetPixels, visibleStartTime, visibleEndTime,
					totalDuration, mousePos, showTooltips, hoveredEntry);
			}
		}

		// Show tooltip for hovered entry
		if(hoveredEntry && showTooltips)
		{
			ImGui::BeginTooltip();
			ImGui::Text("Function: %s", hoveredEntry->section.c_str());
			ImGui::Text("Duration: %s", formatDuration(hoveredEntry->duration, true).c_str());
			ImGui::Text("Percentage: %.2f%%", totalDuration > 0 ? (float)hoveredEntry->duration / totalDuration * 100.0f : 0.0f);
			ImGui::Text("Depth: %u", hoveredEntry->depth);
			ImGui::Text("Thread: %s", g_profiler.GetThreadName(hoveredEntry->threadId).c_str());
			ImGui::Text("Start: %llu", hoveredEntry->timestamp);
			ImGui::EndTooltip();
		}

		// Reserve the space we used
		ImGui::Dummy(ImVec2(canvas_sz.x, graphHeight));

		ImGui::End();
	}

}
