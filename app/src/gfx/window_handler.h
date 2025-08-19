#pragma once

#include "core/core_minimal.h"

#include <SDL2/SDL_events.h>

struct GameState;

class WindowHandler
{
public:
	bool InitWindow(GameState& gameState);
	void InitImGui();
	void InitOpenGL(GameState& gameState);

	void HandleInputs(const SDL_Event& event);

	void ShutdownImGui();
	void ShutdownOpenGL(GameState& gameState);
	void Quit();

private:
};
