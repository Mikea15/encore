#pragma once

#include "core/core_minimal.h"

#include "game_state.h"
#include "assets/texture_manager.h"
#include "editor/editor_widget.h"

class TextureManagerWidget : public EditorWidget
{
public:

	virtual void Run(GameState& rGameStates) override
	{
		if(ImGui::Begin("Texture Manager"))
		{
			TextureManager::TexManagerStats stats = TextureManager::GetInstance().GetStats();
			ImGui::Text("Loaded Textures: %lu", stats.loadedTextures);
		}
		ImGui::End();
	}

private:
};
