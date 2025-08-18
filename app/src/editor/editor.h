#pragma once

#include "core/core_minimal.h"
#include "debug/framerate_widget.h"
#include "debug/memory_widget.h"
#include "editor_profiler.h"
#include "game_state.h"
#include "editor_profiler.h"
#include "gfx/camera_2d.h"
#include "gfx/types.h"

#if ENC_DEBUG
#define ENABLE_EDITOR 1
#else
#define ENABLE_EDITOR 0
#endif


#if ENABLE_EDITOR
class Editor
{
public:
	Editor(GameState& rGameState) 
		: m_rGameState(rGameState)
	{}

	void HandleInput(const SDL_Event& event)
	{
		m_cameraInput = {};

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

			m_editorCamera.zoom += m_cameraInput.z * deltaTime;
			m_editorCamera.zoom = utils::Clamp(m_editorCamera.zoom, 0.01f, 100.0f);

			m_editorCamera.cameraVelocity = glm::mix(m_editorCamera.cameraVelocity,
				Vec2(m_cameraInput.x, m_cameraInput.y) * cameraSpeed,
				m_editorCamera.cameraDamping * deltaTime);
		}
	}

private:
	GameState& m_rGameState;

	Camera2D m_editorCamera;
	Vec3 m_cameraInput;
	ProfilerWindow m_profilerWindow;
};
#endif

