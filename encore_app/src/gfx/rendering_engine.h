#pragma once

#include "core/core_minimal.h"

#include "game_state.h"
#include "sprite_renderer.h"

struct Camera2D;

struct RenderCommand
{
	SpriteFrame frame;
	GLuint textureId;
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
	void NewFrame_ImGui();
	void RenderEditorFrame(GameState& rGameState, Camera2D& camera);
	void EndFrame_ImGui();

	void PushRenderCommand(RenderCommand cmd);
	void ClearRenderCommands();

	void ResizeFramebuffer(GameState& rGameState, i32 width, i32 height);

private:
	void RenderScene(GameState& rGameState, Camera2D& camera);
	void BlitFramebufferToScreen(GameState& rGameState);

	void CreateFramebuffer(GameState& rGameState);
	void ClearFramebuffer(GameState& rGameState);

private:
	SpriteBatchRenderer m_spriteRenderer;
	std::vector<RenderCommand> m_renderCommands;

	Spritesheet m_tileset;
	AnimatedSprite m_animSprite;
};
