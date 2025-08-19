
#pragma once

#include "core/core_minimal.h"

#include "gfx/sprite_renderer.h"

namespace debug 
{
	static void DrawRendererStats(const SpriteBatchRenderer& renderer)
	{
		if (ImGui::Begin("Renderer Stats"))
		{
			SpriteBatchRenderer::Stats stats = renderer.GetStats();

			// AutoClear Arena, since Stack will be free at end of scope.
			ImGui::Text("Draw Calls: %u", stats.drawCalls);
			ImGui::Text("Sprites Drawn: %u", stats.spritesDrawn);
			ImGui::Text("Vertices Drawn: %u", stats.verticesDrawn);

		}
		ImGui::End();
	}
}
