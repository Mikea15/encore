#pragma once

#include "core/core_minimal.h"
#include "debug/extension_imgui.h"
#include "game_state.h"
#include "imgui/imgui.h"
#include "profiler/profiler.h"
#include "utils/string_factory.h"
#include "utils/utils_time.h"

#include <algorithm>
#include <thread>

namespace editor 
{
	class ProfilerWindow
	{
	public:
		void DrawProfilerFlameGraph(const GameState& gameState)
		{
			if(framesHist.size() == 0)
			{
				framesHist.resize(framesHistCount);
			}

			if(ImGui::Begin("Profiler"))
			{
				std::vector<ThreadSafeProfiler::Entry> currentEntries;
				if(m_Options.bPaused)
				{
					currentEntries = frozenEntries;
				}
				else if(m_Options.bShowAllThreads)
				{
					currentEntries = g_profiler.GetAllThreadsEntries();
				}
				else if(m_Options.bHasSelectedThread)
				{
					currentEntries = g_profiler.GetThreadEntries(selectedThreadId);
				}
				else
				{
					currentEntries = g_profiler.GetCurrentThreadEntries();
				}

				// If we're transitioning to paused, capture the current frame
				if(m_Options.bPaused && frozenEntries.empty())
				{
					frozenEntries = m_Options.bShowAllThreads ? g_profiler.GetAllThreadsEntries() : g_profiler.GetCurrentThreadEntries();
				}

				// If unpausing, clear frozen data
				if(!m_Options.bPaused && !frozenEntries.empty())
				{
					frozenEntries.clear();
				}

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

				framesHist[frameCount++ % framesHistCount] = (f32)US_TO_MS(totalDuration);
				
				ImGui::PlotHistogram("Histogram", framesHist.data(), (i32)framesHistCount, 0, nullptr, 0.0f, 150.0f, ImVec2(0, 80.0f));

				u64 timeRange = maxTimestamp - minTimestamp;

				// Options
				if(ImGui::CollapsingHeader("Options"))
				{
					ImGui::SeparatorText("Viewport:");
					ImGui::DragFloat("Min Zoom", &m_Options.minZoom, 0.1f, 0.1f, 4.9f);
					ImGui::DragFloat("Max Zoom", &m_Options.maxZoom, 1.0f, 5.0f, 100.0f);
					
					ImGui::SeparatorText("Style:");
					ImGui::DragFloat("Bar Height", &m_Options.barHeight, 0.1f, 5.0f, 25.0f);
					ImGui::DragFloat("Bar Spacing", &m_Options.barSpacing, 0.1f, 0.0f, 5.0f);
					ImGui::DragFloat("Side Margin", &m_Options.sideMargin, 0.1f, 0.0f, 20.0f);
					ImGui::DragFloat("Top Margin", &m_Options.topMargin, 0.1f, 0.0f, 50.0f);
					ImGui::DragFloat("Thread Separator Height", &m_Options.threadSeparatorHeight, 0.1f, 0.0f, 30.0f);
				}

				// Controls
				if(ImGui::Button(m_Options.bPaused ? "Resume" : "Pause"))
				{
					m_Options.bPaused = !m_Options.bPaused;
					if(m_Options.bPaused)
					{
						frozenEntries = m_Options.bShowAllThreads ? g_profiler.GetAllThreadsEntries() : g_profiler.GetCurrentThreadEntries();
					}
				}
				
				ImGui::SameLine(); ImGui::Checkbox("Show Tooltips", &m_Options.bShowTooltips);
				ImGui::SameLine(); ImGui::Checkbox("Show All Threads", &m_Options.bShowAllThreads);

				// Thread selection dropdown
				if(!m_Options.bShowAllThreads)
				{
					ImGui::SameLine();
					if(ImGui::BeginCombo("Thread", m_Options.bHasSelectedThread ? g_profiler.GetThreadName(selectedThreadId).c_str() : "Select Thread"))
					{
						for(const auto& [threadId, entries] : threadGroups)
						{
							bool isSelected = (m_Options.bHasSelectedThread && selectedThreadId == threadId);
							std::string threadName = g_profiler.GetThreadName(threadId);

							if(ImGui::Selectable(threadName.c_str(), isSelected))
							{
								selectedThreadId = threadId;
								m_Options.bHasSelectedThread = true;
							}

							if(isSelected)
							{
								ImGui::SetItemDefaultFocus();
							}
						}
						ImGui::EndCombo();
					}
				}

				ImGui::Separator();

				ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
				ImVec2 canvas_sz = ImGui::GetContentRegionAvail();

				// Calculate height needed for multi-thread display
				float graphHeight = (maxDepth + 1) * (m_Options.barHeight + m_Options.barSpacing) + m_Options.topMargin + 50.0f;
				if(m_Options.bShowAllThreads && threadGroups.size() > 1)
				{
					// Calculate height per thread group
					float totalHeight = 0;
					for(const auto& [threadId, entries] : threadGroups)
					{
						u32 threadMaxDepth = 2;
						for(const auto& entry : entries)
						{
							threadMaxDepth = std::max(threadMaxDepth, entry.depth);
						}
						totalHeight += (threadMaxDepth + 1) * (m_Options.barHeight + m_Options.barSpacing) + m_Options.threadSeparatorHeight;
					}
					graphHeight += totalHeight;
				}

				canvas_sz.y = std::max(canvas_sz.y, graphHeight);
				ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

				ImU32 bgColor = ImGui::WithAlpha(ImGui::PASTEL_LIGHT_BLUE, 40);
				ImDrawList* pDrawList = ImGui::GetWindowDrawList();
				pDrawList->AddRectFilled(canvas_p0, canvas_p1, bgColor);

				float virtualCanvasWidth = (canvas_sz.x - m_Options.sideMargin * 2) * zoomLevel;
				float viewportWidth = canvas_sz.x - m_Options.sideMargin * 2;

				// Pan offset in pixels
				float maxPanOffset = std::max(0.0f, virtualCanvasWidth - viewportWidth);
				float panOffsetPixels = scrollOffset * maxPanOffset;

				// Convert back to time domain
				float pixelsPerNanosecond = timeRange > 0 ? virtualCanvasWidth / (float)timeRange : 0.0f;
				u64 panOffsetTime = pixelsPerNanosecond > 0 ? (u64)(panOffsetPixels / pixelsPerNanosecond) : 0;

				u64 visibleStartTime = minTimestamp + panOffsetTime;
				u64 visibleTimeRange = zoomLevel > 0 ? (u64)((float)timeRange / zoomLevel) : timeRange;
				u64 visibleEndTime = visibleStartTime + visibleTimeRange;

				ImVec2 mousePos = ImGui::GetMousePos();
				HandlePanCanvas(mousePos, canvas_p0, canvas_p1, virtualCanvasWidth, viewportWidth);

				float currentY = canvas_p0.y + m_Options.topMargin;

				// Reset Hover each Frame.
				pHoveredEntry = nullptr;

				if(m_Options.bShowAllThreads && threadGroups.size() > 1)
				{
					for(auto& [threadId, entries] : threadGroups)
					{
						const u64 threadDuration = threadDurations[threadId];
						std::string threadLabel = g_profiler.GetThreadName(threadId);

						const char* threadInfoStr = StringFactory::TempFormat("%s - Duration: %u ms", threadLabel.c_str(), US_TO_MS(threadDuration));

						ImU32 threadLabelColor = ImGui::WithAlpha(ImGui::PASTEL_LEMON_CHIFFON, 160);
						pDrawList->AddText(ImVec2(canvas_p0.x + m_Options.sideMargin, currentY), threadLabelColor, threadInfoStr);
						currentY += 20.0f;

						for(auto& entry : entries)
						{
							DrawProfileEntry(*pDrawList, entry, canvas_p0, canvas_sz, currentY,
								minTimestamp, timeRange, virtualCanvasWidth, panOffsetPixels, visibleStartTime, visibleEndTime,
								totalDuration, mousePos);
						}

						// Find max depth for this thread to advance Y properly
						u32 threadMaxDepth = 0;
						for(const auto& entry : entries)
						{
							threadMaxDepth = std::max(threadMaxDepth, entry.depth);
						}

						currentY += (threadMaxDepth + 1) * (m_Options.barHeight + m_Options.barSpacing) + m_Options.threadSeparatorHeight;
					}
				}
				else
				{
					// Draw all entries (single thread or filtered)
					for(auto& entry : currentEntries)
					{
						DrawProfileEntry(*pDrawList, entry, canvas_p0, canvas_sz, currentY,
							minTimestamp, timeRange, virtualCanvasWidth, panOffsetPixels, visibleStartTime, visibleEndTime,
							totalDuration, mousePos);
					}
				}

				// Show tooltip for hovered entry
				ShowTooltip(pHoveredEntry, totalDuration);

				// Reserve the space we used
				ImGui::Dummy(ImVec2(canvas_sz.x, graphHeight));

			}
			ImGui::End();
		}

	private:
		void DrawProfileEntry(ImDrawList& rDrawList, ThreadSafeProfiler::Entry& entry, ImVec2 canvas_p0, ImVec2 canvas_sz, float baseY, u64 minTimestamp, u64 timeRange, float virtualCanvasWidth, float panOffsetPixels,
			u64 visibleStartTime, u64 visibleEndTime, u64 totalDuration, ImVec2 mousePos)
		{
			const u64 entryStart = entry.timestamp;
			const u64 entryEnd = entry.timestamp + entry.duration;

			// Cull out of visible range
			if(entryEnd < visibleStartTime || entryStart > visibleEndTime)
			{
				return;
			}

			// Calculate rectangle position and size in virtual space
			if(timeRange == 0)
			{
				return;
			}

			const float virtualStartX = ((float)(entryStart - minTimestamp) / (float)timeRange) * virtualCanvasWidth;
			const float virtualEndX = ((float)(entryEnd - minTimestamp) / (float)timeRange) * virtualCanvasWidth;

			// Apply pan offset and convert to screen space
			float startX = canvas_p0.x + m_Options.sideMargin + virtualStartX - panOffsetPixels;
			float endX = canvas_p0.x + m_Options.sideMargin + virtualEndX - panOffsetPixels;

			float y = baseY + entry.depth * (m_Options.barHeight + m_Options.barSpacing);

			// Clamp to visible area
			startX = std::max(startX, canvas_p0.x + m_Options.sideMargin);
			endX = std::min(endX, canvas_p0.x + canvas_sz.x - m_Options.sideMargin);

			if(endX <= startX)
			{
				return;
			}

			const ImVec2 rectMin(startX, y);
			const ImVec2 rectMax(endX, y + m_Options.barHeight);

			const ImU32 color = ImGui::GetPastelColor(entry.depth + std::hash<std::thread::id>{}(entry.threadId), 80);

			// Draw rectangle
			rDrawList.AddRectFilled(rectMin, rectMax, color);
			rDrawList.AddRect(rectMin, rectMax, ImGui::WithAlpha(color, 220), 0.0f, ImDrawFlags_None, 0.1f);

			// Add text if rectangle is wide enough
			const float rectWidth = endX - startX;
			DrawEntryName(endX, startX, entry, y, rDrawList);

			HandleHoverEntry(mousePos, rectMin, rectMax, entry);
		}

		void DrawEntryName(float endX, float startX, ThreadSafeProfiler::Entry& entry, float y, ImDrawList& rDrawList)
		{
			const float minSizeForText = 50.0f;
			float rectWidth = endX - startX;
			if(rectWidth > minSizeForText)
			{
				std::string text = entry.section;
				ImVec2 textSize = ImGui::CalcTextSize(text.c_str());

				// Truncate text if needed
				const float spaceForText = rectWidth - 10.0f;
				if(textSize.x > spaceForText)
				{
					text = text.substr(0, (size_t)(spaceForText / textSize.x * text.length())) + "...";
				}

				ImVec2 textPos(startX + 2.0f, y + (m_Options.barHeight - textSize.y) * 0.5f);
				rDrawList.AddText(textPos, IM_COL32(255, 255, 255, 255), text.c_str());
			}
		}

		void HandleHoverEntry(ImVec2& mousePos, const ImVec2& rectMin, const ImVec2& rectMax, ThreadSafeProfiler::Entry& entry)
		{
			if(m_Options.bShowTooltips)
			{
				if(mousePos.x >= rectMin.x && mousePos.x <= rectMax.x && mousePos.y >= rectMin.y && mousePos.y <= rectMax.y)
				{
					pHoveredEntry = &entry;
				}
			}
			else
			{
				pHoveredEntry = nullptr;
			}
		}

		void HandlePanCanvas(ImVec2& mousePos, ImVec2& canvasStart, ImVec2& canvasEnd, float virtualCanvasWidth, float viewportWidth)
		{
			const bool bMouseOverCanvas = mousePos.x >= canvasStart.x
				&& mousePos.x <= canvasEnd.x
				&& mousePos.y >= canvasStart.y
				&& mousePos.y <= canvasEnd.y;

			if(bMouseOverCanvas)
			{
				const float wheel = ImGui::GetIO().MouseWheel;
				if(wheel != 0.0f)
				{
					zoomLevel *= (1.0f + wheel * 0.1f);
					zoomLevel = std::clamp(zoomLevel, m_Options.minZoom, m_Options.maxZoom);
				}

				if(ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				{
					bDragging = true;
					dragStartPos = mousePos;
					dragStartScrollOffset = scrollOffset;
				}

				if(ImGui::IsMouseClicked(ImGuiMouseButton_Right))
				{
					zoomLevel = 1.0f;
					scrollOffset = 0.0f;
				}
			}

			if(bDragging)
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
					bDragging = false;
				}
			}
		}

		void ShowTooltip(const ThreadSafeProfiler::Entry* pEntry, u64 totalDuration)
		{
			if(!m_Options.bShowTooltips || pEntry == nullptr)
			{
				return;
			}

			constexpr bool bShowMicroseconds = true;
			ImGui::BeginTooltip();
			ImGui::Text("%s", g_profiler.GetThreadName(pEntry->threadId).c_str());
			ImGui::Text("Task: %s", pEntry->section.c_str());
			ImGui::Text("Duration: %s | %.2f%%", 
				utils::FormatDuration(pEntry->duration, bShowMicroseconds),
				totalDuration > 0 ? (float)pEntry->duration / totalDuration * 100.0f : 0.0f);
			ImGui::EndTooltip();
		}

	private:
		ThreadSafeProfiler::Entry* pHoveredEntry = nullptr;
		std::vector<ThreadSafeProfiler::Entry> frozenEntries; // Store paused data
		std::thread::id selectedThreadId;

		float zoomLevel = 1.0f;
		float scrollOffset = 0.0f;

		ImVec2 dragStartPos;
		float dragStartScrollOffset;
		bool bDragging = false;

		std::vector<float> framesHist;
		const u32 framesHistCount = 240;
		u32 frameCount = 0;

		struct EditorOptions
		{
			// Drawing parameters
			float barHeight = 15.0f;
			float barSpacing = 1.0f;
			float sideMargin = 5.0f;
			float topMargin = 5.0f;
			float threadSeparatorHeight = 15.0f;

			float minZoom = 0.1f;
			float maxZoom = 20.0f;

			bool bShowAllThreads = true;
			bool bGroupByThreads = false;
			bool bHasSelectedThread = false;
			bool bShowTooltips = true;
			bool bPaused = false;
		} m_Options;
	};
}
