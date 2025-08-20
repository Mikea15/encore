#pragma once

#include "core/core_minimal.h"
#include "editor/editor.h"

#include "gfx/rendering_engine.h"
#include "gfx/window_handler.h"
#include "states/state_sandbox.h"
#include "tasks/task_system.h"

class GameEngine
{
public:
	GameEngine();
	bool Run();

private:
	void InitCoreSubsystems();
	void InitMemoryPools();
	void InitGameState();
	void InitGame();

	void RegisterTasks();

	void HandleInput();
	void Update(float deltaTime);
	void Render();

	void ShutdownGameState();

private:
	WindowHandler m_window;
	RenderingEngine m_renderingEngine;
	TaskSchedulerSystem m_taskScheduler;
	GameState m_gameState;


	Editor m_editor;


	SandboxState m_sandbox;

	bool m_bIsRunning = false;
};
