#pragma once

#include "core/core_minimal.h"

#include "debug/memory_widget.h"
#include "debug/profiler_widget.h"
#include "debug/renderer_widget.h"

#include "editor/editor_profiler.h"
#include "game_state.h"
#include "renderer_sprite.h"

struct RenderCommand
{
	Sprite sprite;
	Vec2 position;
	f32 rotation;
};

class RenderingEngine
{
public:
	RenderingEngine(SpriteBatchRenderer& renderer);

	void Init(GameState& rGameState);
	void Shutdown(GameState& rGameState);

	void RenderFrame(GameState& gameState, Camera2D& camera);
	void RenderEditorFrame(GameState& gameState, Camera2D& camera);

	void PushRenderCommand(RenderCommand cmd);
	void ClearRenderCommands();

private:
	void RenderScene(GameState& gameState, Camera2D& camera);
	void RenderImGui(GameState& gameState);
	void BlitFramebufferToScreen(GameState& gameState);

	void CreateFramebuffer(GameState& gameState);
	void ClearFramebuffer(GameState& gameState);
	void ResizeFramebuffer(GameState& gameState, i32 width, i32 height);

private:
	editor::ProfilerWindow m_profilerWindow;
	SpriteBatchRenderer& m_2dRenderer;
	std::vector<RenderCommand> m_renderCommands;
};
