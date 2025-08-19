#pragma once

#include "core/core_minimal.h"

#include "game_state.h"
#include "debug/extension_imgui.h"
#include "imgui/imgui.h"
#include "profiler/profiler.h"
#include "utils/string_factory.h"
#include "utils/utils_math.h"
#include "utils/utils_time.h"

#include <algorithm>
#include <thread>


class ProfilerWindow
{
public:
	void DrawProfilerFlameGraph(const GameState& gameState)
	{
		m_frameEntries.clear();

		if(ImGui::Begin("Profiler"))
		{
			if(m_options.bPaused)
			{
				m_frameEntries = frozenEntries;
			}
			else if(m_options.bShowAllThreads)
			{
				m_frameEntries = Profiler::GetInstance().GetAllThreadsEntries();
			}
			else if(m_options.bHasSelectedThread)
			{
				m_frameEntries = Profiler::GetInstance().GetThreadEntries(selectedThreadId);
			}
			else
			{
				m_frameEntries = Profiler::GetInstance().GetCurrentThreadEntries();
			}

			// If we're transitioning to paused, capture the current frame
			if(m_options.bPaused && frozenEntries.empty())
			{
				frozenEntries = m_options.bShowAllThreads ? Profiler::GetInstance().GetAllThreadsEntries() : Profiler::GetInstance().GetCurrentThreadEntries();
			}

			// If unpausing, clear frozen data
			if(!m_options.bPaused && !frozenEntries.empty())
			{
				frozenEntries.clear();
			}

			u64 totalDuration = 0;
			u64 minTimestamp = UINT64_MAX;
			u64 maxTimestamp = 0;
			u8 maxDepth = 0;

			// Group m_frameEntries by thread for better analysis
			std::unordered_map<std::thread::id, std::vector<ProfilerEntry>> threadGroups;
			std::unordered_map<std::thread::id, u64> threadDurations;

			for(const auto& entry : m_frameEntries)
			{
				threadGroups[entry.threadId].push_back(entry);

				if(entry.depth == 0)
				{
					threadDurations[entry.threadId] += entry.duration;
					totalDuration += entry.duration;
				}

				minTimestamp = utils::Min(minTimestamp, entry.timestamp);
				maxTimestamp = utils::Max(maxTimestamp, entry.timestamp + entry.duration);
				maxDepth = utils::Max(maxDepth, entry.depth);
			}

			u64 timeRange = maxTimestamp - minTimestamp;

			// Options
			if(ImGui::CollapsingHeader("Options"))
			{
				ImGui::SeparatorText("Viewport:");
				ImGui::DragFloat("Min Zoom", &m_options.minZoom, 0.1f, 0.1f, 4.9f);
				ImGui::DragFloat("Max Zoom", &m_options.maxZoom, 1.0f, 5.0f, 100.0f);

				ImGui::SeparatorText("Style:");
				ImGui::DragFloat("Bar Height", &m_options.barHeight, 0.1f, 5.0f, 25.0f);
				ImGui::DragFloat("Bar Spacing", &m_options.barSpacing, 0.1f, 0.0f, 5.0f);
				ImGui::DragFloat("Side Margin", &m_options.sideMargin, 0.1f, 0.0f, 20.0f);
				ImGui::DragFloat("Top Margin", &m_options.topMargin, 0.1f, 0.0f, 50.0f);
				ImGui::DragFloat("Thread Separator Height", &m_options.threadSeparatorHeight, 0.1f, 0.0f, 30.0f);
				ImGui::DragInt("Entry Background Opacity", &m_options.entryBackgroundOpacity, 1, 0, 255);
				ImGui::DragInt("Entry Border Opacity", &m_options.entryBorderOpacity, 1, 0, 255);
			}

			// Controls
			if(ImGui::Button(m_options.bPaused ? "Resume" : "Pause"))
			{
				m_options.bPaused = !m_options.bPaused;
				if(m_options.bPaused)
				{
					frozenEntries = m_options.bShowAllThreads ? Profiler::GetInstance().GetAllThreadsEntries() : Profiler::GetInstance().GetCurrentThreadEntries();
				}
			}

			ImGui::SameLine(); ImGui::Checkbox("Show Tooltips", &m_options.bShowTooltips);
			ImGui::SameLine(); ImGui::Checkbox("Group by Threads", &m_options.bGroupByThreads);
			ImGui::SameLine(); ImGui::Checkbox("Show All Threads", &m_options.bShowAllThreads);

			// Thread selection dropdown
			if(!m_options.bShowAllThreads)
			{
				ImGui::SameLine();
				if(ImGui::BeginCombo("Thread", m_options.bHasSelectedThread ? Profiler::GetInstance().GetThreadName(selectedThreadId) : "Select Thread"))
				{
					for(const auto& [threadId, entries] : threadGroups)
					{
						bool isSelected = (m_options.bHasSelectedThread && selectedThreadId == threadId);
						std::string threadName = Profiler::GetInstance().GetThreadName(threadId);

						if(ImGui::Selectable(threadName.c_str(), isSelected))
						{
							selectedThreadId = threadId;
							m_options.bHasSelectedThread = true;
						}

						if(isSelected)
						{
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}
			}

			if(ImGui::BeginTable("MAIN_DISPLAY", 2, ImGuiTableFlags_ScrollY | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable))
			{
				ImGui::TableNextRow();

				ImGui::TableNextColumn();
				{
					ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
					ImVec2 canvas_sz = ImGui::GetContentRegionAvail();

					// Calculate height needed for multi-thread display
					float graphHeight = (maxDepth + 1) * (m_options.barHeight + m_options.barSpacing) + m_options.topMargin + 50.0f;
					if(m_options.bShowAllThreads && threadGroups.size() > 1)
					{
						// Calculate height per thread group
						float totalHeight = 0;
						for(const auto& [threadId, entries] : threadGroups)
						{
							u8 threadMaxDepth = 2;
							for(const auto& entry : entries)
							{
								threadMaxDepth = utils::Max(threadMaxDepth, entry.depth);
							}
							totalHeight += (threadMaxDepth + 1) * (m_options.barHeight + m_options.barSpacing) + m_options.threadSeparatorHeight;
						}
						graphHeight += totalHeight;
					}

					canvas_sz.y = std::max(canvas_sz.y, graphHeight);
					ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

					ImU32 bgColor = ImGui::WithAlpha(ImGui::PASTEL_LIGHT_BLUE, 40);
					ImDrawList* pDrawList = ImGui::GetWindowDrawList();
					pDrawList->AddRectFilled(canvas_p0, canvas_p1, bgColor);

					float virtualCanvasWidth = (canvas_sz.x - m_options.sideMargin * 2) * m_zoomLevel;
					float viewportWidth = canvas_sz.x - m_options.sideMargin * 2;

					// Pan offset in pixels
					float maxPanOffset = std::max(0.0f, virtualCanvasWidth - viewportWidth);
					float panOffsetPixels = m_scrollOffset * maxPanOffset;

					// Convert back to time domain
					float pixelsPerNanosecond = timeRange > 0 ? virtualCanvasWidth / (float)timeRange : 0.0f;
					u64 panOffsetTime = pixelsPerNanosecond > 0 ? (u64)(panOffsetPixels / pixelsPerNanosecond) : 0;

					u64 visibleStartTime = minTimestamp + panOffsetTime;
					u64 visibleTimeRange = m_zoomLevel > 0 ? (u64)((float)timeRange / m_zoomLevel) : timeRange;
					u64 visibleEndTime = visibleStartTime + visibleTimeRange;

					ImVec2 mousePos = ImGui::GetMousePos();
					HandlePanCanvas(mousePos, canvas_p0, canvas_p1, virtualCanvasWidth, viewportWidth);

					float currentY = canvas_p0.y + m_options.topMargin;

					// Reset Hover each Frame.
					pHoveredEntry = nullptr;

					if(m_options.bShowAllThreads && threadGroups.size() > 1)
					{
						for(auto& [threadId, entries] : threadGroups)
						{
							const u64 threadDuration = threadDurations[threadId];
							std::string threadLabel = Profiler::GetInstance().GetThreadName(threadId);

							const char* threadInfoStr = StringFactory::TempFormat("%s - Duration: %u ms", threadLabel.c_str(), US_TO_MS(threadDuration));

							ImU32 threadLabelColor = ImGui::WithAlpha(ImGui::PASTEL_LEMON_CHIFFON, 160);
							pDrawList->AddText(ImVec2(canvas_p0.x + m_options.sideMargin, currentY), threadLabelColor, threadInfoStr);
							currentY += 20.0f;

							for(auto& entry : entries)
							{
								DrawProfileEntry(*pDrawList, entry, canvas_p0, canvas_sz, currentY,
									minTimestamp, timeRange, virtualCanvasWidth, panOffsetPixels, visibleStartTime, visibleEndTime,
									totalDuration, mousePos);
							}

							// Find max depth for this thread to advance Y properly
							u8 threadMaxDepth = 0;
							for(const auto& entry : entries)
							{
								threadMaxDepth = utils::Max(threadMaxDepth, entry.depth);
							}

							currentY += (threadMaxDepth + 1) * (m_options.barHeight + m_options.barSpacing) + m_options.threadSeparatorHeight;
						}
					}
					else
					{
						// Draw all entries (single thread or filtered)
						for(auto& entry : m_frameEntries)
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


				ImGui::TableNextColumn();
				{

					// Display frame time and thread breakdown
					ImGui::Text("FrameTime: %.3f ms", US_TO_MS((f32)totalDuration));

					if(m_options.bShowAllThreads && threadDurations.size() > 1)
					{
						ImGui::Text("Threads: %zu", threadDurations.size());

						// Show thread time breakdown
						for(const auto& [threadId, duration] : threadDurations)
						{
							ImGui::Text("  %s: %.2f ms (%.1f%%)",
								Profiler::GetInstance().GetThreadName(threadId),
								US_TO_MS((f32)duration),
								totalDuration > 0 ? (f32)duration / totalDuration * 100.0f : 0.0f);
						}
						ImGui::Separator();
					}

					if(ImGui::BeginTable("PROFILER_TABLE", m_options.bGroupByThreads && m_options.bShowAllThreads ? 3 : 2,
						ImGuiTableFlags_Sortable | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Borders))
					{
						ImGui::TableSetupScrollFreeze(0, 1);
						ImGui::TableSetupColumn("Function", ImGuiTableColumnFlags_WidthStretch);
						if(m_options.bGroupByThreads && m_options.bShowAllThreads)
						{
							ImGui::TableSetupColumn("Thread", ImGuiTableColumnFlags_WidthFixed, 100.0f);
						}
						ImGui::TableSetupColumn("Duration", ImGuiTableColumnFlags_WidthStretch);
						ImGui::TableHeadersRow();

						// Sort m_frameEntries by duration (descending)
						std::vector<ProfilerEntry> sortedEntries = m_frameEntries;
						std::sort(sortedEntries.begin(), sortedEntries.end(),
							[](const ProfilerEntry& a, const ProfilerEntry& b) {
								return a.duration > b.duration;
							});

						if(m_options.bGroupByThreads && m_options.bShowAllThreads && threadGroups.size() > 1)
						{
							// Display grouped by thread
							for(const auto& [threadId, threadEntries] : threadGroups)
							{
								// Thread header row
								ImGui::TableNextRow();
								ImGui::TableNextColumn();

								ImU32 headerColor = ImGui::GetColorU32(ImGuiCol_HeaderActive);
								ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, headerColor);
								ImGui::Text("%s", Profiler::GetInstance().GetThreadName(threadId));

								ImGui::TableNextColumn();
								ImGui::Text("Thread");
								ImGui::TableNextColumn();
								ImGui::Text("%.2f ms", US_TO_MS((f32)threadDurations[threadId]));

								// Sort thread m_frameEntries
								std::vector<ProfilerEntry> sortedThreadEntries = threadEntries;
								std::sort(sortedThreadEntries.begin(), sortedThreadEntries.end(),
									[](const ProfilerEntry& a, const ProfilerEntry& b) {
										return a.threadId > b.threadId;
									});

								// Display m_frameEntries for this thread
								for(const auto& entry : sortedThreadEntries)
								{
									ImGui::TableNextRow();
									ImGui::TableNextColumn();

									// Indent based on depth
									std::string indent(entry.depth * 2, ' ');
									ImGui::Text("%s%s", indent.c_str(), entry.section);

									ImGui::TableNextColumn();
									ImGui::Text("%s", Profiler::GetInstance().GetThreadName(entry.threadId));

									ImGui::TableNextColumn();
									const char* str = StringFactory::TempFormat("%.3f ms", US_TO_MS((f32)entry.duration));
									const float percentage = totalDuration > 0 ? (float)entry.duration / totalDuration : 0.0f;
									ImGui::UsageProgressBar(str, percentage, ImVec2(-1.0f, 15.0f));
								}
							}
						}
						else
						{
							// Display all m_frameEntries in one list
							for(const auto& entry : sortedEntries)
							{
								ImGui::TableNextRow();
								ImGui::TableNextColumn();

								// Indent based on depth
								std::string indent(entry.depth * 2, ' ');
								ImGui::Text("%s%s", indent.c_str(), entry.section);

								if(m_options.bGroupByThreads && m_options.bShowAllThreads)
								{
									ImGui::TableNextColumn();
									ImGui::Text("%s", Profiler::GetInstance().GetThreadName(entry.threadId));
								}

								ImGui::TableNextColumn();

								const char* str = StringFactory::TempFormat("%.3f ms", US_TO_MS((f32)entry.duration));
								const float percentage = totalDuration > 0 ? (float)entry.duration / totalDuration : 0.0f;
								ImGui::UsageProgressBar(str, percentage, ImVec2(-1.0f, 15.0f));
							}
						}

						ImGui::EndTable();
					}

					// Additional statistics
					if(m_frameEntries.size() > 0)
					{
						ImGui::Separator();
						ImGui::Text("Total Entries: %zu", m_frameEntries.size());

						if(m_options.bShowAllThreads && threadDurations.size() > 1)
						{
							// Find heaviest thread
							auto heaviestThread = std::max_element(threadDurations.begin(), threadDurations.end(),
								[](const auto& a, const auto& b) { return a.second < b.second; });

							if(heaviestThread != threadDurations.end())
							{
								ImGui::Text("Heaviest Thread: %s (%.2f ms)",
									Profiler::GetInstance().GetThreadName(heaviestThread->first),
									US_TO_MS((f32)heaviestThread->second));
							}
						}
					}
				}

				ImGui::EndTable();
			}

		}
		ImGui::End();
	}

private:
	void DrawProfileEntry(ImDrawList& rDrawList, ProfilerEntry& entry, ImVec2 canvas_p0, ImVec2 canvas_sz, float baseY, u64 minTimestamp, u64 timeRange, float virtualCanvasWidth, float panOffsetPixels,
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
		float startX = canvas_p0.x + m_options.sideMargin + virtualStartX - panOffsetPixels;
		float endX = canvas_p0.x + m_options.sideMargin + virtualEndX - panOffsetPixels;

		float y = baseY + entry.depth * (m_options.barHeight + m_options.barSpacing);

		// Clamp to visible area
		startX = std::max(startX, canvas_p0.x + m_options.sideMargin);
		endX = std::min(endX, canvas_p0.x + canvas_sz.x - m_options.sideMargin);

		if(endX <= startX)
		{
			return;
		}

		const ImVec2 rectMin(startX, y);
		const ImVec2 rectMax(endX, y + m_options.barHeight);

		const ImU32 backgroundColor = ImGui::GetPastelColor(entry.depth + std::hash<std::thread::id>{}(entry.threadId), m_options.entryBackgroundOpacity);

		// Draw rectangle
		rDrawList.AddRectFilled(rectMin, rectMax, backgroundColor);
		rDrawList.AddRect(rectMin, rectMax, ImGui::WithAlpha(backgroundColor, m_options.entryBorderOpacity), 0.0f, ImDrawFlags_None, 0.1f);

		// Add text if rectangle is wide enough
		const float rectWidth = endX - startX;
		DrawEntryName(endX, startX, entry, y, rDrawList);

		HandleHoverEntry(mousePos, rectMin, rectMax, entry);
	}

	void DrawEntryName(float endX, float startX, ProfilerEntry& entry, float y, ImDrawList& rDrawList)
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

			ImVec2 textPos(startX + 2.0f, y + (m_options.barHeight - textSize.y) * 0.5f);
			rDrawList.AddText(textPos, IM_COL32(255, 255, 255, 255), text.c_str());
		}
	}

	void HandleHoverEntry(ImVec2& mousePos, const ImVec2& rectMin, const ImVec2& rectMax, ProfilerEntry& entry)
	{
		if(m_options.bShowTooltips)
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
				m_zoomLevel *= (1.0f + wheel * 0.1f);
				m_zoomLevel = std::clamp(m_zoomLevel, m_options.minZoom, m_options.maxZoom);
			}

			if(ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				m_bDragging = true;
				m_dragStartPos = mousePos;
				m_dragStartScrollOffset = m_scrollOffset;
			}

			if(ImGui::IsMouseClicked(ImGuiMouseButton_Right))
			{
				m_zoomLevel = 1.0f;
				m_scrollOffset = 0.0f;
			}
		}

		if(m_bDragging)
		{
			if(ImGui::IsMouseDown(ImGuiMouseButton_Left))
			{
				ImVec2 currentMousePos = ImGui::GetMousePos();
				float dragDelta = currentMousePos.x - m_dragStartPos.x;

				float maxPanOffset = std::max(0.0f, virtualCanvasWidth - viewportWidth);
				if(maxPanOffset > 0.0f)
				{
					float scrollDelta = -dragDelta / maxPanOffset;
					m_scrollOffset = std::clamp(m_dragStartScrollOffset + scrollDelta, 0.0f, 1.0f);
				}
			}
			else
			{
				m_bDragging = false;
			}
		}
	}

	void ShowTooltip(const ProfilerEntry* pEntry, u64 totalDuration)
	{
		if(!m_options.bShowTooltips || pEntry == nullptr)
		{
			return;
		}

		constexpr bool bShowMicroseconds = true;
		ImGui::BeginTooltip();
		ImGui::Text("%s", Profiler::GetInstance().GetThreadName(pEntry->threadId));
		ImGui::Text("Task: %s", pEntry->section);
		ImGui::Text("Duration: %s | %.2f%%",
			utils::FormatDuration(pEntry->duration, bShowMicroseconds),
			totalDuration > 0 ? (float)pEntry->duration / totalDuration * 100.0f : 0.0f);
		ImGui::EndTooltip();
	}


private:
	ProfilerEntry* pHoveredEntry = nullptr;
	std::vector<ProfilerEntry> frozenEntries; // Store paused data
	std::thread::id selectedThreadId;
	std::vector<ProfilerEntry> m_frameEntries;

	float m_zoomLevel = 1.0f;
	float m_scrollOffset = 0.0f;

	ImVec2 m_dragStartPos;
	float m_dragStartScrollOffset;
	bool m_bDragging = false;

	struct EditorOptions
	{
		// Drawing parameters
		float barHeight = 15.0f;
		float barSpacing = 1.0f;
		float sideMargin = 5.0f;
		float topMargin = 5.0f;
		float threadSeparatorHeight = 15.0f;

		float minZoom = 0.01f;
		float maxZoom = 100.0f;

		i32 entryBackgroundOpacity = 60;
		i32 entryBorderOpacity = 180;

		bool bShowAllThreads = true;
		bool bGroupByThreads = false;
		bool bHasSelectedThread = false;
		bool bShowTooltips = true;
		bool bPaused = false;
	} m_options;
};

