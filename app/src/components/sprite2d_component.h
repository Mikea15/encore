
#pragma once

#include "core/core_minimal.h"

#include "components/move_component.h"
#include "memory/base_pool.h"
#include "renderer/render2d_types.h"
#include "utils/utils_rand.h"

struct Sprite2DComponent
{
public:
	DECLARE_POOL(Sprite2DComponent);

	Sprite2DComponent() = default;
	Sprite2DComponent(const Sprite& sprite, MoveComponent& rMoveComponent)
		: m_sprite(sprite)
		, m_rMoveComponent(rMoveComponent)
	{}

	const Sprite& GetSprite() const { return m_sprite; }
	const MoveComponent& GetMovementComponent() const { return m_rMoveComponent; }

private:
	Sprite m_sprite;
	MoveComponent& m_rMoveComponent;
};

