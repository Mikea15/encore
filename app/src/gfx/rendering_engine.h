#pragma once

#include "core/core_minimal.h"

#include "editor/editor_profiler.h"
#include "renderer_imgui.h"
#include "sprite_renderer.h"

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
	RenderingEngine();

	void Init(GameState& rGameState);
	void Shutdown(GameState& rGameState);

	void RenderFrame(GameState& rGameState, Camera2D& camera);
	void RenderEditorFrame(GameState& rGameState, Camera2D& camera);

	void PushRenderCommand(RenderCommand cmd);
	void ClearRenderCommands();

private:
	void RenderScene(GameState& rGameState, Camera2D& camera);
	void BlitFramebufferToScreen(GameState& rGameState);

	void CreateFramebuffer(GameState& rGameState);
	void ClearFramebuffer(GameState& rGameState);
	void ResizeFramebuffer(GameState& rGameState, i32 width, i32 height);

private:
	ProfilerWindow m_profilerWindow;
	SpriteBatchRenderer m_spriteRenderer;
	std::vector<RenderCommand> m_renderCommands;
	RendererImGui m_imguiRenderer;
};
