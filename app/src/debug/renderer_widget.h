
#pragma once

#include "core/core_minimal.h"

#include "extension_imgui.h"

#include "memory/base_arena.h"
#include "renderer/renderer_sprite.h"

namespace debug 
{
	static void DrawRendererStats(Render2DInfo& renderer)
	{
		if (ImGui::Begin("Renderer Stats"))
		{
			SpriteBatchRenderer::Stats stats = renderer.renderer.GetStats();

			// AutoClear Arena, since Stack will be free at end of scope.
			ImGui::Text("Draw Calls: %u", stats.drawCalls);
			ImGui::Text("Sprites Drawn: %u", stats.spritesDrawn);
			ImGui::Text("Vertices Drawn: %u", stats.verticesDrawn);

		}
		ImGui::End();
	}
}
