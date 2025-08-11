
#pragma once

#include "core/core_minimal.h"

#include "memory/base_pool.h"
#include "renderer/render2d_types.h"
#include "utils/utils_rand.h"

struct Sprite2DComponent
{
public:
	DECLARE_POOL(Sprite2DComponent);

	Sprite2DComponent() = default;
	Sprite2DComponent(const Sprite& sprite)
		: m_sprite(sprite)
	{}

	const Sprite& GetSprite() const { return m_sprite; }

private:
	Sprite m_sprite;
};

