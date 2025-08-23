#pragma once

#include "core/core_minimal.h"

#include "editor/editor.h"
#include "gfx/rendering_engine.h"
#include "gfx/window_handler.h"
#include "integrations/livepp_handler.h"
#include "states/state_sandbox.h"
#include "tasks/task_system.h"

enum class RuntimeMode : u8
{
	Game,
	Editor,

	Count
};

class GameEngine
{
public:
	GameEngine();
	bool Run();

private:
	void InitGameState();
	void InitCoreSubsystems();
	void InitGame();

	void RegisterTasks();

	void HandleInput();
	void Update(float deltaTime);
	void Render();

	void ShutdownGameState();

	void CycleRuntimeMode();

	RuntimeMode GetRuntimeMode() const { return (RuntimeMode) m_runtimeMode; }
	bool IsEditorRuntimeMode() const { return GetRuntimeMode() == RuntimeMode::Editor; }
	bool IsGameRuntimeMode() const { return GetRuntimeMode() == RuntimeMode::Game; }

private:
	WindowHandler m_window;
	RenderingEngine m_renderingEngine;
	TaskSchedulerSystem m_taskScheduler;
	GameState m_gameState;

	Editor m_editor;

	SandboxState m_sandbox;

#if USE_LPP
	LivePPHandler m_lppHandler;
#endif

	u8 m_runtimeMode;

	bool m_bIsRunning = false;
};
