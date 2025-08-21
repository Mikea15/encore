#pragma once

#include "core/core_minimal.h"

#include <SDL2/SDL_events.h>
#include "gfx/camera_2d.h"
#include <vector>

struct GameState;
class RenderingEngine;
class EditorWidget;

class Editor
{
public:
	void Init(GameState* pGameState, RenderingEngine* pRenderingEngine);
	void Shutdown();

	void HandleInput(const SDL_Event& event);
	void Update(float deltaTime);

	void RenderEditor();
	void RenderWidgets();

	Camera2D& GetCamera() { return m_camera; }

private:
	GameState* m_pGameState = nullptr;
	RenderingEngine* m_pRenderingEngine = nullptr;

	Camera2D m_camera;
	Vec3 m_cameraInput = {};

	std::vector<EditorWidget*> m_editorWidgets;
};


