#pragma once

#include "core/core_minimal.h"
#include "components/move_component.h"
#include "components/sprite2d_component.h"
#include "memory/base_pool.h"
#include "renderer/render2d_types.h"
#include "utils/utils_rand.h"

class Entity
{
public:
	Entity(const Vec2& position, float rotation, const Sprite& sprite)
	{
		m_pMoveComponent = MoveComponent::Alloc(position, rotation);
		m_pSpriteComponent = Sprite2DComponent::Alloc(sprite);
	}

	const MoveComponent& GetMoveComponent() const { Assert(m_pMoveComponent); return *m_pMoveComponent; }
	const Sprite2DComponent& GetSprite2DComponent() const { Assert(m_pSpriteComponent); return *m_pSpriteComponent; }

private:
	Vec2 m_position;
	MoveComponent* m_pMoveComponent;
	Sprite2DComponent* m_pSpriteComponent;
};

