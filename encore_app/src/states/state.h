#pragma once

#include "core/core_minimal.h"

#include "SDL2/SDL_events.h"

class State
{
public:
	virtual ~State() = default;

	virtual void Init() {};
	virtual void Shutdown() {};
	virtual void HandleInput(const SDL_Event& event) {};
	virtual void Update(float deltaTime) {};
	virtual void Render() {};
};
