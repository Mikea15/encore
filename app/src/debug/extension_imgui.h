#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

namespace ImGui 
{
	// Enhanced pastel colors with more saturation and contrast while maintaining softness
	static constexpr ImU32 PASTEL_LIGHT_BLUE = IM_COL32(120, 180, 220, 255);      // More saturated blue
	static constexpr ImU32 PASTEL_SKY_BLUE = IM_COL32(100, 170, 230, 255);        // Deeper sky blue
	static constexpr ImU32 PASTEL_LIGHT_STEEL_BLUE = IM_COL32(140, 170, 210, 255); // More contrast
	static constexpr ImU32 PASTEL_LAVENDER = IM_COL32(200, 180, 240, 255);        // More purple tint
	static constexpr ImU32 PASTEL_PALE_TURQUOISE = IM_COL32(120, 220, 210, 255);  // More vibrant turquoise
	static constexpr ImU32 PASTEL_LIGHT_GREEN = IM_COL32(120, 220, 120, 255);     // More saturated green
	static constexpr ImU32 PASTEL_PALE_GREEN = IM_COL32(130, 240, 130, 255);      // Brighter pale green
	static constexpr ImU32 PASTEL_HONEYDEW = IM_COL32(220, 250, 220, 255);        // Slightly more green
	static constexpr ImU32 PASTEL_MINT_CREAM = IM_COL32(220, 250, 230, 255);      // More minty
	static constexpr ImU32 PASTEL_LIGHT_PINK = IM_COL32(250, 150, 170, 255);      // More saturated pink
	static constexpr ImU32 PASTEL_PEACH_PUFF = IM_COL32(250, 200, 150, 255);      // Warmer peach
	static constexpr ImU32 PASTEL_PINK = IM_COL32(250, 160, 180, 255);            // More vibrant pink
	static constexpr ImU32 PASTEL_MISTY_ROSE = IM_COL32(250, 210, 210, 255);      // Slightly more rose
	static constexpr ImU32 PASTEL_ANTIQUE_WHITE = IM_COL32(240, 220, 190, 255);   // Warmer antique
	static constexpr ImU32 PASTEL_LIGHT_YELLOW = IM_COL32(250, 250, 180, 255);    // More saturated yellow
	static constexpr ImU32 PASTEL_LEMON_CHIFFON = IM_COL32(250, 240, 170, 255);   // More lemony
	static constexpr ImU32 PASTEL_PAPAYA_WHIP = IM_COL32(250, 220, 180, 255);     // Warmer papaya
	static constexpr ImU32 PASTEL_OLD_LACE = IM_COL32(245, 230, 200, 255);        // Warmer lace
	static constexpr ImU32 PASTEL_PLUM = IM_COL32(200, 120, 200, 255);            // More saturated plum
	static constexpr ImU32 PASTEL_ORCHID = IM_COL32(210, 90, 200, 255);           // More vibrant orchid
	static constexpr ImU32 PASTEL_LAVENDER_BLUSH = IM_COL32(250, 220, 230, 255);  // More blush
	static constexpr ImU32 PASTEL_VIOLET = IM_COL32(220, 100, 220, 255);          // More saturated violet
	static constexpr ImU32 PASTEL_LIGHT_GOLDENROD = IM_COL32(240, 230, 140, 255); // More golden
	static constexpr ImU32 PASTEL_BISQUE = IM_COL32(250, 210, 170, 255);          // Warmer bisque
	static constexpr ImU32 PASTEL_SEASHELL = IM_COL32(250, 230, 210, 255);        // Warmer seashell
	static constexpr ImU32 PASTEL_BEIGE = IM_COL32(230, 220, 180, 255);           // More contrast beige
	static constexpr ImU32 PASTEL_CORNSILK = IM_COL32(250, 240, 190, 255);        // Warmer cornsilk

	// Randomized enhanced pastel color palette
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

	// NEW: Helper function to create a darker version of any pastel for better contrast
	static ImU32 GetPastelDark(size_t index, float darknessFactor = 0.7f)
	{
		ImU32 color = PASTEL_COLORS[index % PASTEL_COLORS_COUNT];
		u8 r = (u8)(((color >> 0) & 0xFF) * darknessFactor);
		u8 g = (u8)(((color >> 8) & 0xFF) * darknessFactor);
		u8 b = (u8)(((color >> 16) & 0xFF) * darknessFactor);
		u8 a = (color >> 24) & 0xFF;
		return IM_COL32(r, g, b, a);
	}

	// NEW: Helper function to create a lighter version of any pastel
	static ImU32 GetPastelLight(size_t index, float lightnessFactor = 1.2f)
	{
		ImU32 color = PASTEL_COLORS[index % PASTEL_COLORS_COUNT];
		u8 r = (u8)ImMin(255, (int)(((color >> 0) & 0xFF) * lightnessFactor));
		u8 g = (u8)ImMin(255, (int)(((color >> 8) & 0xFF) * lightnessFactor));
		u8 b = (u8)ImMin(255, (int)(((color >> 16) & 0xFF) * lightnessFactor));
		u8 a = (color >> 24) & 0xFF;
		return IM_COL32(r, g, b, a);
	}

	// Enhanced usage progress bar with better color transitions
	static void UsageProgressBar(const char* label, float fraction, ImVec2 size = ImVec2(-1.0f, 0.0f))
	{
		// More vibrant color progression for better visibility
		ImVec4 color;
		if(fraction < 0.25f)
		{
			color = ImVec4(0.2f, 0.9f, 0.3f, 1.0f); // Vibrant green
		}
		else if(fraction < 0.5f)
		{
			color = ImVec4(0.7f, 0.9f, 0.2f, 1.0f); // Bright lime
		}
		else if(fraction < 0.75f)
		{
			color = ImVec4(1.0f, 0.6f, 0.1f, 1.0f); // Vivid orange
		}
		else
		{
			color = ImVec4(0.9f, 0.2f, 0.2f, 1.0f); // Bright red
		}

		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, color);
		ImGui::ProgressBar(fraction, size, label);
		ImGui::PopStyleColor();
	}

}
