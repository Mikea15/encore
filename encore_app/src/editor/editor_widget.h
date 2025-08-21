#pragma once

#include "core/core_minimal.h"

#include <game_state.h>

class EditorWidget
{
public:
	virtual void Run(GameState& rGameState) = 0;
};
