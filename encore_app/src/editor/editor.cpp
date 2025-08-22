#include "editor.h"

#include "editor_widget.h"
#include "game_state.h"
#include "gfx/camera_2d.h"
#include "gfx/frame_stats.h"
#include "gfx/rendering_engine.h"
#include "gfx/types.h"
#include "profiler/profiler_section.h"
#include "widgets/gpu_stats_widget.h"
#include "widgets/memory_monitor_widget.h"
#include "widgets/menu_widget.h"
#include "widgets/performance_monitor_mini_widget.h"
#include "widgets/performance_monitor_widget.h"
#include "widgets/profiler_widget.h"
#include "widgets/scene_viewport_widget.h"

void Editor::Init(GameState* pGameState, RenderingEngine* pRenderingEngine)
{
	m_pGameState = pGameState;
	Assert(m_pGameState);
	m_pRenderingEngine = pRenderingEngine;
	Assert(m_pRenderingEngine);

	m_editorWidgets.push_back(new SceneViewportWidget(pRenderingEngine));
	m_editorWidgets.push_back(new MenuWidget());
	m_editorWidgets.push_back(new MemoryMonitorWidget());
	m_editorWidgets.push_back(new GpuStatsWidget());
	m_editorWidgets.push_back(new ProfilerWidget());
	m_editorWidgets.push_back(new PerformanceMonitorWidget());
	m_editorWidgets.push_back(new PerformanceMonitorMiniWidget());
}

void Editor::Shutdown()
{
	for(auto* pWidget : m_editorWidgets)
	{
		delete pWidget;
	}
	m_editorWidgets.clear();
}

void Editor::HandleInput(const SDL_Event& event)
{
	m_cameraInput = {};

	switch(event.type)
	{
	case SDL_KEYDOWN:
		if(event.key.keysym.sym == SDLK_TAB)
		{
			m_pGameState->editor.bShowImGui = !m_pGameState->editor.bShowImGui;
		}
		break;
	default: break;
	}

	const u8* keyboardState = SDL_GetKeyboardState(nullptr);

	if(keyboardState[SDL_SCANCODE_A]) { m_cameraInput.x = 1.0f; }
	if(keyboardState[SDL_SCANCODE_D]) { m_cameraInput.x = -1.0f; }
	if(keyboardState[SDL_SCANCODE_S]) { m_cameraInput.y = 1.0f; }
	if(keyboardState[SDL_SCANCODE_W]) { m_cameraInput.y = -1.0f; }
	if(keyboardState[SDL_SCANCODE_Q]) { m_cameraInput.z = 1.0f; }
	if(keyboardState[SDL_SCANCODE_E]) { m_cameraInput.z = -1.0f; }

	if(m_cameraInput != Vec3(0.0f))
	{
		m_cameraInput = glm::normalize(m_cameraInput);
	}
}

void Editor::Update(float deltaTime)
{
	PROFILE();

	{
		PROFILE_SCOPE("Editor Camera Update");

		m_camera.zoom += m_cameraInput.z * deltaTime;
		m_camera.zoom = utils::Clamp(m_camera.zoom, 0.01f, 100.0f);

		m_camera.velocity = glm::mix(m_camera.velocity,
			Vec2(m_cameraInput.x, m_cameraInput.y) * m_options.camSpeed,
			m_camera.damping * deltaTime);

		m_camera.position += m_camera.velocity * deltaTime;
	}
}

void Editor::RenderEditor()
{
	PROFILE();
	Assert(m_pGameState);

	// Create a fullscreen dockspace
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
	window_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	ImGui::Begin("EditorDockspace", nullptr, window_flags);
	ImGui::PopStyleVar(3);

	// DockSpace
	ImGuiID dockSpaceId = ImGui::GetID("MainDockSpace");
	ImGui::DockSpace(dockSpaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

	// Render all registered widgets
	RenderWidgets();

	// Rendering
	// debug::DrawRendererStats(m_spriteRenderer);

	// Demo window
	if(m_pGameState->editor.bShowDemoWindow)
	{
		ImGui::ShowDemoWindow(&m_pGameState->editor.bShowDemoWindow);
	}

	ImGui::End();
}

void Editor::RenderWidgets()
{
	for(auto* pWidget : m_editorWidgets)
	{
		pWidget->Run(*m_pGameState);
	}
}
