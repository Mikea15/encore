#pragma once

#include "imgui/imgui.h"

#include <string>

namespace ImGui 
{
	// Individual pastel color constants (callable by name)
	static constexpr ImU32 PASTEL_LIGHT_BLUE = IM_COL32(173, 216, 230, 255);
	static constexpr ImU32 PASTEL_SKY_BLUE = IM_COL32(135, 206, 235, 255);
	static constexpr ImU32 PASTEL_LIGHT_STEEL_BLUE = IM_COL32(176, 196, 222, 255);
	static constexpr ImU32 PASTEL_LAVENDER = IM_COL32(230, 230, 250, 255);
	static constexpr ImU32 PASTEL_PALE_TURQUOISE = IM_COL32(175, 238, 238, 255);
	static constexpr ImU32 PASTEL_LIGHT_GREEN = IM_COL32(144, 238, 144, 255);
	static constexpr ImU32 PASTEL_PALE_GREEN = IM_COL32(152, 251, 152, 255);
	static constexpr ImU32 PASTEL_HONEYDEW = IM_COL32(240, 255, 240, 255);
	static constexpr ImU32 PASTEL_MINT_CREAM = IM_COL32(245, 255, 250, 255);
	static constexpr ImU32 PASTEL_LIGHT_PINK = IM_COL32(255, 182, 193, 255);
	static constexpr ImU32 PASTEL_PEACH_PUFF = IM_COL32(255, 218, 185, 255);
	static constexpr ImU32 PASTEL_PINK = IM_COL32(255, 192, 203, 255);
	static constexpr ImU32 PASTEL_MISTY_ROSE = IM_COL32(255, 228, 225, 255);
	static constexpr ImU32 PASTEL_ANTIQUE_WHITE = IM_COL32(250, 235, 215, 255);
	static constexpr ImU32 PASTEL_LIGHT_YELLOW = IM_COL32(255, 255, 224, 255);
	static constexpr ImU32 PASTEL_LEMON_CHIFFON = IM_COL32(255, 250, 205, 255);
	static constexpr ImU32 PASTEL_PAPAYA_WHIP = IM_COL32(255, 239, 213, 255);
	static constexpr ImU32 PASTEL_OLD_LACE = IM_COL32(253, 245, 230, 255);
	static constexpr ImU32 PASTEL_PLUM = IM_COL32(221, 160, 221, 255);
	static constexpr ImU32 PASTEL_ORCHID = IM_COL32(218, 112, 214, 255);
	static constexpr ImU32 PASTEL_LAVENDER_BLUSH = IM_COL32(255, 240, 245, 255);
	static constexpr ImU32 PASTEL_VIOLET = IM_COL32(238, 130, 238, 255);
	static constexpr ImU32 PASTEL_LIGHT_GOLDENROD = IM_COL32(250, 250, 210, 255);
	static constexpr ImU32 PASTEL_BISQUE = IM_COL32(255, 228, 196, 255);
	static constexpr ImU32 PASTEL_SEASHELL = IM_COL32(255, 245, 238, 255);
	static constexpr ImU32 PASTEL_BEIGE = IM_COL32(245, 245, 220, 255);
	static constexpr ImU32 PASTEL_CORNSILK = IM_COL32(255, 248, 220, 255);

	// Randomized pastel color palette (no gradients on sequential access)
	static const ImU32 PASTEL_COLORS[] = {
		PASTEL_ORCHID,
		PASTEL_LIGHT_GREEN,
		PASTEL_BISQUE,
		PASTEL_SKY_BLUE,
		PASTEL_MISTY_ROSE,
		PASTEL_PALE_TURQUOISE,
		PASTEL_LIGHT_GOLDENROD,
		PASTEL_PLUM,
		PASTEL_HONEYDEW,
		PASTEL_PEACH_PUFF,
		PASTEL_LAVENDER,
		PASTEL_PALE_GREEN,
		PASTEL_SEASHELL,
		PASTEL_LIGHT_STEEL_BLUE,
		PASTEL_VIOLET,
		PASTEL_ANTIQUE_WHITE,
		PASTEL_MINT_CREAM,
		PASTEL_LIGHT_PINK,
		PASTEL_LEMON_CHIFFON,
		PASTEL_BEIGE,
		PASTEL_LAVENDER_BLUSH,
		PASTEL_LIGHT_BLUE,
		PASTEL_PAPAYA_WHIP,
		PASTEL_PINK,
		PASTEL_OLD_LACE,
		PASTEL_LIGHT_YELLOW,
		PASTEL_CORNSILK,
	};

	// Get the number of colors in the palette
	static constexpr u32 PASTEL_COLORS_COUNT = sizeof(PASTEL_COLORS) / sizeof(PASTEL_COLORS[0]);

	// Helper function to get a color by index (with wrapping)
	static ImU32 GetPastelColor(size_t index)
	{
		return PASTEL_COLORS[index % PASTEL_COLORS_COUNT];
	}

	// Helper function to get a color by index with custom alpha
	static ImU32 GetPastelColor(size_t index, u8 alpha)
	{
		ImU32 color = PASTEL_COLORS[index % PASTEL_COLORS_COUNT];
		return (color & 0x00FFFFFF) | (alpha << 24);
	}

	// Helper function to get a color based on a hash (for consistent coloring)
	static ImU32 GetPastelColorByHash(const std::string& str)
	{
		std::hash<std::string> hasher;
		size_t hash = hasher(str);
		return PASTEL_COLORS[hash % PASTEL_COLORS_COUNT];
	}

	// Helper function to get a color based on a hash with custom alpha
	static ImU32 GetPastelColorByHash(const std::string& str, u8 alpha)
	{
		std::hash<std::string> hasher;
		size_t hash = hasher(str);
		ImU32 color = PASTEL_COLORS[hash % PASTEL_COLORS_COUNT];
		return (color & 0x00FFFFFF) | (alpha << 24);
	}

	// Helper function to get a color based on depth (cycles through colors)
	static ImU32 GetPastelColorByDepth(u32 depth)
	{
		return PASTEL_COLORS[depth % PASTEL_COLORS_COUNT];
	}

	// Helper function to get a color based on depth with custom alpha
	static ImU32 GetPastelColorByDepth(u32 depth, u8 alpha)
	{
		ImU32 color = PASTEL_COLORS[depth % PASTEL_COLORS_COUNT];
		return (color & 0x00FFFFFF) | (alpha << 24);
	}

	// Helper function to modify alpha of any color
	static ImU32 WithAlpha(ImU32 color, u8 alpha)
	{
		return (color & 0x00FFFFFF) | (alpha << 24);
	}

	static void UsageProgressBar(const char* label, float fraction, ImVec2 size = ImVec2(-1.0f, 0.0f))
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
