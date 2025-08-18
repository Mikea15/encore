#pragma once

#include "core/core_minimal.h"

#include "state.h"

class SandboxState : public State
{
public:
	virtual void Init() override
	{
		Texture texSlimeIdle = TextureManager::GetInstance().LoadFromFile("../assets/sprites/slime1_temp.png");

		// Create some test sprites
		for(int i = 0; i < 100000; ++i)
		{
			Sprite sprite;
			sprite.position = {
				(rand() % 2000) - 1000.0f, // Random X: -1000 to 1000
				(rand() % 2000) - 1000.0f // Random Y: -1000 to 1000
			};
			sprite.size = { (rand() % 32) + 1.0f, (rand() % 32) + 1.0f };
			//sprite.color = {
			//	(rand() % 255) / 255.0f, // Random color
			//	(rand() % 255) / 255.0f,
			//	(rand() % 255) / 255.0f,
			//	1.0f
			//};
			sprite.texId = texSlimeIdle.GetTextureID();

			Entity* pEnt = Entity::Alloc();
			Assert(pEnt);
			pEnt->RegisterComponents(sprite.position, utils::GetFloat(0.0f, 360.0f), sprite);
		}
	}

	virtual void HandleInput(const SDL_Event& event) override
	{

	}
};
