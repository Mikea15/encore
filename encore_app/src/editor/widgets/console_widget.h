#pragma once
#include "core/core_minimal.h"

#include "game_state.h"
#include "editor/editor_widget.h"
#include "profiler/profiler.h"
#include "profiler/profiler_section.h"


class ConsoleWidget : public EditorWidget
{
public:
	virtual void Run(GameState& rGameStates) override
	{
		if(ImGui::Begin("Console"))
		{
			PROFILE_SCOPE("Console");
			ImGui::Text("Console Output");
			ImGui::Separator();
			ImGui::Text("Application running...");
			ImGui::Text("Scene rendering to texture: %dx%d", rGameStates.framebufferWidth, rGameStates.framebufferHeight);
		}
		ImGui::End();
	}
};
