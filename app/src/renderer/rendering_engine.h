#pragma once

#include "core/core_minimal.h"
#include "editor/editor_profiler.h"
#include "renderer_imgui.h"

class SpriteBatchRenderer;
struct Camera2D;

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
	void BlitFramebufferToScreen(GameState& gameState);

	void CreateFramebuffer(GameState& gameState);
	void ClearFramebuffer(GameState& gameState);
	void ResizeFramebuffer(GameState& gameState, i32 width, i32 height);

private:
	editor::ProfilerWindow m_profilerWindow;
	SpriteBatchRenderer& m_2dRenderer;
	std::vector<RenderCommand> m_renderCommands;
	RendererImGui m_imguiRenderer;
};
