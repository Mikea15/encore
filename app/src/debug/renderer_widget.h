
#pragma once

#include "imgui/imgui.h"
#include "extension_imgui.h"
#include "core_minimal.h"
#include "base_arena.h"

#include "../renderer/renderer_sprite.h"

namespace debug 
{
	void DrawRendererStats(Render2D& renderer)
	{
		if (ImGui::Begin("Renderer Stats"))
		{
			SpriteBatchRenderer::Stats stats = renderer.renderer.GetStats();

			// AutoClear Arena, since Stack will be free at end of scope.
			ImGui::Text("Draw Calls: %u", stats.draw_calls);
			ImGui::Text("Sprites Drawn: %u", stats.sprites_drawn);
			ImGui::Text("Vertices Drawn: %u", stats.vertices_drawn);

		}
		ImGui::End();
	}
}
