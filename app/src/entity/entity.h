#pragma once

#include "core/core_minimal.h"
#include "components/move_component.h"
#include "memory/base_pool.h"
#include "renderer/render2d_types.h"
#include "utils/utils_rand.h"

class Entity
{
public:
	Entity()
		: m_position()
	{
		m_pMoveComponent = MoveComponent::Alloc();
	}

	void Init(Sprite sprite)
	{
		m_pMoveComponent->Init(sprite);
	}

	const MoveComponent& GetMoveComponent() const { Assert(m_pMoveComponent); return *m_pMoveComponent; }

private:
	Vec2 m_position;
	MoveComponent* m_pMoveComponent;
};

