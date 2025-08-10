#include "game_state.h"

GameState CreateDefaultGameState()
{
	GameState gameState;
	// Memory
	gameState.arenas[AT_GLOBAL] = arena_create(KILOBYTES(24));
	gameState.arenas[AT_COMPONENTS] = arena_create(MEGABYTES(50));
	gameState.arenas[AT_FRAME] = arena_create(MEGABYTES(1));

	// Default window settings.
	gameState.window.width = 1280;
	gameState.window.height = 720;
	return gameState;
}

void ClearGameState(GameState& gameState)
{
	for(int i = 0; i < AT_COUNT; i++)
	{
		arena_destroy(&gameState.arenas[i]);
	}
}
