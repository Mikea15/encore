#pragma once

#include "core/core_minimal.h"

#include "extension_imgui.h"
#include "memory/base_arena.h"
#include "utils/string_factory.h"

namespace debug 
{
	static void DrawMemoryStats(Arena& arena, const char* name) 
	{
		if (ImGui::Begin("Memory Stats")) 
		{
			ArenaStats stats = arena_get_stats(&arena);
			
			const char* str = StringFactory::TempFormat(
				"\"%s\">> %.1f%% ( Used: %.2f KB | Free: %.2f KB )",
				name, 
				stats.usageRatio, 
				(f32)BYTES_TO_KB(stats.usedBytes), 
				(f32)BYTES_TO_KB(stats.freeBytes));

			ImGui::UsageProgressBar(str, stats.usageRatio / 100.0f, ImVec2(-1.0f, 15.0f));

		}
		ImGui::End();
	}

	// Pool usage widget - displays usage stats and visual progress bar
	template<typename T>
	void DrawPoolUsageWidget(const char* pPoolName, Pool<T>* pPool, bool bShowDetails = true)
	{
		if(!pPool)
		{
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s: Pool Invalid", pPoolName);
			return;
		}

		u32 capacity = pPool->GetCapacity();
		u32 activeCount = pPool->GetActiveCount();
		u32 freeCount = pPool->GetFreeCount();
		float usagePercent = pPool->GetUsagePercentage();

		// Choose color based on usage
		ImVec4 barColor;
		if(usagePercent < 50.0f)
			barColor = ImVec4(0.0f, 0.8f, 0.0f, 1.0f);  // Green - healthy
		else if(usagePercent < 70.0f)
			barColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);  // Yellow - caution
		else if(usagePercent < 90.0f)
			barColor = ImVec4(1.0f, 0.6f, 0.0f, 1.0f);  // Orange - warning
		else
			barColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);  // Red - critical

		// Pool name and main stats
		ImGui::Text("%s", pPoolName);
		ImGui::SameLine();
		ImGui::TextColored(barColor, "(%.1f%%)", usagePercent);

		// Progress bar with usage visualization
		char overlayText[64];
		snprintf(overlayText, sizeof(overlayText), "%u / %u", activeCount, capacity);
		ImGui::ProgressBar(usagePercent / 100.0f, ImVec2(-1.0f, 0.0f), overlayText);

		// Optional detailed stats
		if(bShowDetails)
		{
			ImGui::Indent();
			ImGui::Text("Active: %u", activeCount);
			ImGui::Text("Free: %u", freeCount);
			ImGui::Text("Capacity: %u", capacity);

			// Memory usage estimation
			size_t itemSize = sizeof(T);
			size_t totalMemory = capacity * (itemSize + sizeof(u32) + sizeof(b8));
			if(totalMemory > 1024 * 1024)
				ImGui::Text("Memory: %.2f MB", (float)totalMemory / (1024.0f * 1024.0f));
			else if(totalMemory > 1024)
				ImGui::Text("Memory: %.2f KB", (float)totalMemory / 1024.0f);
			else
				ImGui::Text("Memory: %zu bytes", totalMemory);

			ImGui::Unindent();
		}
	}

	// Compact version for dashboard-style display
	template<typename T>
	void DrawPoolUsageCompact(const char* pPoolName, Pool<T>* pPool)
	{
		if(!pPool) return;

		float usagePercent = pPool->GetUsagePercentage();
		u32 activeCount = pPool->GetActiveCount();
		u32 capacity = pPool->GetCapacity();

		// Color coding
		ImVec4 color;
		if(usagePercent < 70.0f)
			color = ImVec4(0.0f, 0.8f, 0.0f, 1.0f);
		else if(usagePercent < 90.0f)
			color = ImVec4(1.0f, 0.6f, 0.0f, 1.0f);
		else
			color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("%s", pPoolName);

		ImGui::TableSetColumnIndex(1);
		ImGui::TextColored(color, "%u/%u", activeCount, capacity);

		ImGui::TableSetColumnIndex(2);
		ImGui::ProgressBar(usagePercent / 100.0f, ImVec2(-1.0f, 0.0f), "");

		ImGui::TableSetColumnIndex(3);
		ImGui::TextColored(color, "%.1f%%", usagePercent);
	}

	// Multi-pool dashboard
	class PoolDashboard
	{
	public:
		static void BeginDashboard(const char* pTitle = "Pool Usage Dashboard")
		{
			if(ImGui::Begin(pTitle))
			{
				if(ImGui::BeginTable("PoolTable", 4, ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders))
				{
					ImGui::TableSetupColumn("Pool Name", ImGuiTableColumnFlags_WidthFixed, 120.0f);
					ImGui::TableSetupColumn("Usage", ImGuiTableColumnFlags_WidthFixed, 80.0f);
					ImGui::TableSetupColumn("Progress", ImGuiTableColumnFlags_WidthStretch);
					ImGui::TableSetupColumn("Percent", ImGuiTableColumnFlags_WidthFixed, 60.0f);
					ImGui::TableHeadersRow();
				}
			}
		}

		template<typename T>
		static void AddPool(const char* pPoolName, Pool<T>* pPool)
		{
			DrawPoolUsageCompact(pPoolName, pPool);
		}

		static void EndDashboard()
		{
			ImGui::EndTable();
			ImGui::End();
		}
	};
}

// Macro to easily add pools created with DECLARE_POOL/IMPLEMENT_POOL
#define IMGUI_DRAW_POOL_WIDGET(PoolType, bShowDetails) \
    debug::DrawPoolUsageWidget(#PoolType, PoolType::GetPool(), bShowDetails)

#define IMGUI_ADD_POOL_TO_DASHBOARD(PoolType) \
    debug::PoolDashboard::AddPool(#PoolType, PoolType::GetPool())
