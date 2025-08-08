
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
					(void)sprintf_s(buff, "%.2f ms", NsToMilliseconds((f32)entry.duration));
					ImGui::UsageProgressBar(buff, (float)entry.duration / totalTime, ImVec2(-1.0f, 15.0f));

					ImGui::TableNextRow();
				}
			}
			ImGui::EndTable();
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
				return std::to_string(duration) + " μs";
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

	static void DrawProfilerFlameGraph()
	{
		static bool showTooltips = true;
		static float zoomLevel = 1.0f;
		static float scrollOffset = 0.0f;
		static bool isPaused = false;
		static std::vector<Profiler::Entry> frozenEntries; // Store paused data

		if(!ImGui::Begin("Flame Graph"))
		{
			ImGui::End();
			return;
		}

		// Get current data or use frozen data if paused
		const std::vector<Profiler::Entry>& currentEntries = isPaused ? frozenEntries : g_profiler.lastEntries;

		// If we're transitioning to paused, capture the current frame
		if(isPaused && frozenEntries.empty())
		{
			frozenEntries = g_profiler.lastEntries;
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

		for(const auto& entry : currentEntries)
		{
			if(entry.depth == 0)
				totalDuration += entry.duration;

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
				frozenEntries = g_profiler.lastEntries;
			}
		}
		ImGui::SameLine();
		ImGui::Checkbox("Show Tooltips", &showTooltips);

		ImGui::Text("Frame Time: %.3f ms %s", NsToMilliseconds((f32)totalDuration), isPaused ? "(PAUSED)" : "");

		ImGui::Separator();

		// Get drawing area
		ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
		ImVec2 canvas_sz = ImGui::GetContentRegionAvail();

		// Drawing parameters
		float barHeight = 20.0f;
		float barSpacing = 2.0f;
		float leftMargin = 10.0f;
		float topMargin = 40.0f; // Increased to make room for header line

		// Reserve space for the flame graph
		float graphHeight = (maxDepth + 1) * 25.0f + 50.0f; // 25px per depth level + padding
		canvas_sz.y = std::min(canvas_sz.y, graphHeight);

		ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

		// Draw background
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 50));
		draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 160));

		// Calculate virtual canvas size and visible viewport
		float virtualCanvasWidth = (canvas_sz.x - leftMargin * 2) * zoomLevel; // Virtual area gets bigger when zoomed
		float viewportWidth = canvas_sz.x - leftMargin * 2; // Actual visible area stays same

		// Pan offset in pixels (scrollOffset controls how much we've panned)
		float maxPanOffset = std::max(0.0f, virtualCanvasWidth - viewportWidth);
		float panOffsetPixels = scrollOffset * maxPanOffset;

		// Convert back to time domain
		float pixelsPerNanosecond = virtualCanvasWidth / (float)timeRange;
		u64 panOffsetTime = (u64)(panOffsetPixels / pixelsPerNanosecond);

		u64 visibleStartTime = minTimestamp + panOffsetTime;
		u64 visibleTimeRange = (u64)((float)timeRange / zoomLevel);
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

		// Draw the line with text in the middle
		float headerTextWidth = ImGui::CalcTextSize(headerText).x;
		float availableWidth = canvas_sz.x - leftMargin * 2;
		float lineLength = (availableWidth - headerTextWidth - 20) / 2; // 20px padding around text

		// Left line
		draw_list->AddLine(
			ImVec2(canvas_p0.x + leftMargin, lineY - 7),
			ImVec2(canvas_p0.x + leftMargin + lineLength, lineY - 7),
			IM_COL32(255, 255, 255, 255), 1.0f
		);

		// Text in the middle
		float textX = canvas_p0.x + leftMargin + lineLength + 10;
		draw_list->AddText(ImVec2(textX, headerY), IM_COL32(255, 255, 255, 50), headerText);

		// Right line
		draw_list->AddLine(
			ImVec2(textX + headerTextWidth + 10, lineY - 7),
			ImVec2(canvas_p0.x + canvas_sz.x - leftMargin, lineY - 7),
			IM_COL32(255, 255, 255, 255), 1.0f
		);

		// Handle mouse interactions for panning and zooming
		ImVec2 mousePos = ImGui::GetMousePos();
		const Profiler::Entry* hoveredEntry = nullptr;
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

				// Convert pixel drag to scroll offset
				float virtualCanvasWidth = (canvas_sz.x - leftMargin * 2) * zoomLevel;
				float viewportWidth = canvas_sz.x - leftMargin * 2;
				float maxPanOffset = std::max(0.0f, virtualCanvasWidth - viewportWidth);

				if(maxPanOffset > 0.0f)
				{
					float scrollDelta = -dragDelta / maxPanOffset; // Negative for natural drag direction
					scrollOffset = std::clamp(dragStartScrollOffset + scrollDelta, 0.0f, 1.0f);
				}
			}
			else
			{
				isDragging = false;
			}
		}

		for(const auto& entry : currentEntries)
		{
			// Skip entries outside visible time range
			u64 entryStart = entry.timestamp;
			u64 entryEnd = entry.timestamp + entry.duration;

			if(entryEnd < visibleStartTime || entryStart > visibleEndTime)
				continue;

			// Calculate rectangle position and size in virtual space
			float virtualStartX = ((float)(entryStart - minTimestamp) / (float)timeRange) * virtualCanvasWidth;
			float virtualEndX = ((float)(entryEnd - minTimestamp) / (float)timeRange) * virtualCanvasWidth;

			// Apply pan offset and convert to screen space
			float startX = canvas_p0.x + leftMargin + virtualStartX - panOffsetPixels;
			float endX = canvas_p0.x + leftMargin + virtualEndX - panOffsetPixels;

			float y = canvas_p0.y + topMargin + entry.depth * (barHeight + barSpacing);

			// Clamp to visible area
			startX = std::max(startX, canvas_p0.x + leftMargin);
			endX = std::min(endX, canvas_p1.x - leftMargin);

			if(endX <= startX) continue; // Skip if too narrow

			ImVec2 rectMin(startX, y);
			ImVec2 rectMax(endX, y + barHeight);

			// Color based on duration percentage  
			ImU32 color = ImGui::GetPastelColor(entry.depth, 128);

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
				draw_list->AddText(textPos, IM_COL32(0, 0, 0, 255), text.c_str());
			}

			// Check for mouse hover
			if(showTooltips &&
				mousePos.x >= rectMin.x && mousePos.x <= rectMax.x &&
				mousePos.y >= rectMin.y && mousePos.y <= rectMax.y)
			{
				hoveredEntry = &entry;
			}
		}

		// Show tooltip for hovered entry
		if(hoveredEntry && showTooltips)
		{
			ImGui::BeginTooltip();
			ImGui::Text("Function: %s", hoveredEntry->section.c_str());
			ImGui::Text("Duration: %s", formatDuration(hoveredEntry->duration, true).c_str());
			ImGui::Text("Percentage: %.2f%%", (float)hoveredEntry->duration / totalDuration * 100.0f);
			ImGui::Text("Depth: %u", hoveredEntry->depth);
			ImGui::Text("Start: %llu", hoveredEntry->timestamp);
			ImGui::EndTooltip();
		}



		// Reserve the space we used
		ImGui::Dummy(ImVec2(canvas_sz.x, graphHeight));

		ImGui::End();
	}
}
