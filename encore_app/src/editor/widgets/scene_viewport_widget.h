#pragma once

#include "core/core_minimal.h"

#include "game_state.h"
#include "editor/editor_widget.h"
#include "gfx/rendering_engine.h"
#include "profiler/profiler.h"
#include "profiler/profiler_section.h"
#include "utils/utils_math.h"


class SceneViewportWidget : public EditorWidget
{
public:
	SceneViewportWidget(RenderingEngine* pRenderingEngine)
		: m_pRenderingEngine(pRenderingEngine)
	{}

	virtual void Run(GameState& rGameStates) override
	{
		if(ImGui::Begin("Scene Viewport"))
		{
			PROFILE_SCOPE("Scene ViewPort");
			ImVec2 content_region = ImGui::GetContentRegionAvail();
			content_region.x = utils::Max(64.0f, content_region.x);
			content_region.y = utils::Max(64.0f, content_region.y);

			// Resize framebuffer if needed
			m_pRenderingEngine->ResizeFramebuffer(rGameStates, (i32)content_region.x, (i32)content_region.y);

			// Display the rendered scene texture
			ImGui::Image((void*)(intptr_t)rGameStates.colorTexture,
				ImVec2((f32)rGameStates.framebufferWidth, (f32)rGameStates.framebufferHeight), ImVec2(0, 1), ImVec2(1, 0));
		}
		ImGui::End();
	}

private:
	RenderingEngine* m_pRenderingEngine = nullptr;
};
