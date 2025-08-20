#include "game_engine.h"

#include "entity/entity.h"
#include "gfx/frame_stats.h"
#include "imgui/backends/imgui_impl_sdl2.h"
#include "profiler/profiler_section.h"

GameEngine::GameEngine()
	: m_window()
	, m_renderingEngine()
	, m_gameState()
	, m_editor()
{
}

bool GameEngine::Run()
{
	InitGameState();
	InitCoreSubsystems();
	InitMemoryPools();
	InitGame();

	RegisterTasks();

	f32 deltaTime = 0.0f;
	f32 lastUpdate = 0u;

	// Main Loop
	while (m_bIsRunning)
	{
		PROFILE_FRAME_START_ALL_THREADS();

#ifdef USE_LPP
		{
			PROFILE_SCOPE("LPP_SyncPoint");
			lppHandler.SyncPoint();
		}
#endif

		// Calculate delta time
		const f32 timeNow = (f32)SDL_GetTicks64() / 1000.0f;
		deltaTime = static_cast<f32>(timeNow - lastUpdate);
		lastUpdate = timeNow;

		// Save Arena checkpoint
		ARENA_SAVE(&m_gameState.arenas[AT_FRAME]);

		HandleInput();
		Update(deltaTime);
		Render();

		// Reset Frame Arena
		ARENA_RESET(&m_gameState.arenas[AT_FRAME]);
	}

	m_renderingEngine.Shutdown(m_gameState);

#ifdef USE_LPP
	lppHandler.Clear();
#endif

	m_window.ShutdownImGui();
	ShutdownGameState();

	m_window.Quit();
	return m_bIsRunning;
}

void GameEngine::InitCoreSubsystems()
{
	StringFactory::Init(&m_gameState.arenas[AT_GLOBAL], &m_gameState.arenas[AT_FRAME]);

#if ENC_DEBUG
	frame_stats_init(g_frameStats, m_gameState.arenas[AT_GLOBAL]);
#endif
}

void GameEngine::InitMemoryPools()
{
	Entity::Init(&m_gameState.arenas[AT_COMPONENTS]);
	MoveComponent::Init(&m_gameState.arenas[AT_COMPONENTS]);
	Sprite2DComponent::Init(&m_gameState.arenas[AT_COMPONENTS]);
}

void GameEngine::RegisterTasks()
{
	auto clearRenderTask = m_taskScheduler.CreateTask("ClearRenderCommand List", [this](float deltaTime) {
		m_renderingEngine.ClearRenderCommands();
		});

	auto moveTask = m_taskScheduler.CreateTask("MoveComponent Pool", [this](float deltaTime) {
		// Rotate Sprites
		Pool<MoveComponent>* pMovePool = MoveComponent::GetPool();
		AssertMsg(pMovePool, "Call MoveComponent::InitPool() first");
		for(MoveComponent& moveComp : *pMovePool)
		{
			moveComp.Update(deltaTime);
		}
		});

	moveTask->AddDependency(clearRenderTask);

	auto pushRenderTask = m_taskScheduler.CreateTask("Sprite2DComponent Pool", [this](float deltaTime) {
		Pool<Sprite2DComponent>* pSpritePool = Sprite2DComponent::GetPool();
		AssertMsg(pSpritePool, "Call MoveComponent::InitPool() first");
		RenderCommand cmd;
		for(Sprite2DComponent& comp : *pSpritePool)
		{
			// Get the entity this component belongs to
			Entity* pEntity = Entity::GetPool()->Get(comp.GetEntityId());
			if(!pEntity) continue;

			MoveComponent* pMoveComp = pEntity->GetMoveComponent();
			if(!pMoveComp) continue;

			cmd.sprite = comp.GetSprite();
			cmd.position = pMoveComp->GetPosition();
			cmd.rotation = pMoveComp->GetRotation();
			m_renderingEngine.PushRenderCommand(cmd);
		}
		});

	pushRenderTask->AddDependency(clearRenderTask);
	pushRenderTask->AddDependency(moveTask);


	m_taskScheduler.CreateExecutionPlan();
}

void GameEngine::HandleInput()
{
	PROFILE();

	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		// Handle Input
		ImGui_ImplSDL2_ProcessEvent(&event);

		switch (event.type)
		{
		case SDL_QUIT:
			m_bIsRunning = false;
			break;
		case SDL_WINDOWEVENT:
			if (event.window.event == SDL_WINDOWEVENT_CLOSE
				&& event.window.windowID == SDL_GetWindowID(m_gameState.window.pWindow))
			{
				m_bIsRunning = false;
			}
			if (event.window.event == SDL_WINDOWEVENT_RESIZED)
			{
				m_gameState.window.width = event.window.data1;
				m_gameState.window.height = event.window.data2;
			}
			break;
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_ESCAPE)
			{
				m_bIsRunning = false;
			}
			break;
		default: break;
		}

		m_editor.HandleInput(event);
	}
}

void GameEngine::Update(float deltaTime)
{
	PROFILE();

#if ENC_DEBUG
	frame_stats_update(g_frameStats, deltaTime);
#endif

	// TASK_GRAPH
	m_taskScheduler.ExecuteTaskGraph(deltaTime);

	m_editor.Update(deltaTime);
}

void GameEngine::Render()
{
	PROFILE();
	if (m_gameState.editor.bShowImGui)
	{
		m_renderingEngine.RenderEditorFrame(m_gameState, m_editor.GetCamera());
	}
	else
	{
		m_renderingEngine.RenderFrame(m_gameState, m_editor.GetCamera());
	}
}

void GameEngine::ShutdownGameState()
{
	for (int i = 0; i < AT_COUNT; i++)
	{
		arena_destroy(&m_gameState.arenas[i]);
	}
}

void GameEngine::InitGameState()
{
	m_gameState.arenas[AT_GLOBAL] = arena_create(KILOBYTES(24));
	m_gameState.arenas[AT_COMPONENTS] = arena_create(MEGABYTES(50));
	m_gameState.arenas[AT_FRAME] = arena_create(MEGABYTES(1));

	// Default window settings.
	m_gameState.window.width = 1280;
	m_gameState.window.height = 720;
}

void GameEngine::InitGame()
{
	// Init Window
	if (!m_window.InitWindow(m_gameState))
	{
		LOG_ERROR("Could not Init Window");
		return;
	}

	m_window.InitImGui();
	m_window.InitOpenGL(m_gameState);

#if !ENC_RELEASE
	PROFILE_SET_THREAD_NAME("MainThread");
#endif

#ifdef USE_LPP
	LivePPHandler lppHandler;
	lppHandler.InitSynchedAgent();
#endif

	m_editor.Init(&m_gameState);

	m_renderingEngine.Init(m_gameState);

	// State
	m_sandbox.Init();

	m_bIsRunning = true;
}
