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
		Assert(m_pMoveComponent);
		// Ugly. Fix later.
		// Use u16 ids, instead of pool pointers.
		// or extract location to entity instead of move component.
		// each comp should know about the entity as well.
		// entity can be a pool too, might as well.
		// that way each comp, can keep an id to the entity as well.
		m_pSpriteComponent = Sprite2DComponent::Alloc(sprite, *m_pMoveComponent);
	}

	const MoveComponent& GetMoveComponent() const { Assert(m_pMoveComponent); return *m_pMoveComponent; }
	const Sprite2DComponent& GetSprite2DComponent() const { Assert(m_pSpriteComponent); return *m_pSpriteComponent; }

private:
	Vec2 m_position;
	MoveComponent* m_pMoveComponent;
	Sprite2DComponent* m_pSpriteComponent;
};

