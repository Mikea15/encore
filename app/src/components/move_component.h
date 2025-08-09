
#pragma once

#include "core/core_minimal.h"

#include "memory/base_pool.h"
#include "renderer/render2d_types.h"
#include "utils/utils_rand.h"

struct MoveComponent
{
public:
	DECLARE_POOL(MoveComponent);

	MoveComponent() {}

	void Init(Sprite sprite) {
		m_sprite = sprite;
	}

	void Update(float deltaTime)
	{
		m_sprite.rotation += deltaTime * rand() * 0.03f;
	}

	Sprite m_sprite;
};

