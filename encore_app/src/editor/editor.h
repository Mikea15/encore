#pragma once

#include "core/core_minimal.h"

#include "editor_profiler.h"
#include "game_state.h"
#include "debug/memory_widget.h"
#include "gfx/camera_2d.h"
#include "gfx/types.h"
#include "profiler/profiler_section.h"

#if ENC_DEBUG
#define ENABLE_EDITOR 1
#else
#define ENABLE_EDITOR 0
#endif


#if ENABLE_EDITOR
class Editor
{
public:
	void Init(GameState* pGameState)
	{
		m_pGameState = pGameState;
	}
	void HandleInput(const SDL_Event& event)
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

	void Update(float deltaTime)
	{
		PROFILE();

		const float cameraSpeed = 500.0f;
		if(m_cameraInput != Vec3(0.0f))
		{
			PROFILE_SCOPE("Editor Camera Update");

			m_cameraInput = glm::normalize(m_cameraInput);

			m_camera.zoom += m_cameraInput.z * deltaTime;
			m_camera.zoom = utils::Clamp(m_camera.zoom, 0.01f, 100.0f);

			m_camera.cameraVelocity = glm::mix(m_camera.cameraVelocity,
				Vec2(m_cameraInput.x, m_cameraInput.y) * cameraSpeed,
				m_camera.cameraDamping * deltaTime);
		}
	}

	Camera2D& GetCamera() { return m_camera; }

private:
	GameState* m_pGameState = nullptr;

	Camera2D m_camera;
	Vec3 m_cameraInput;
	ProfilerWindow m_profilerWindow;
};
#endif

