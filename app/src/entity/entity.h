#pragma once

#include "core/core_minimal.h"
#include "components/move_component.h"
#include "components/sprite2d_component.h"
#include "memory/base_pool.h"
#include "gfx/types.h"
#include "utils/utils_rand.h"

class Entity : public PoolId
{
public:
	DECLARE_POOL(Entity);

	Entity() = default;

	void RegisterComponents(const Vec2& position, float rotation, const Sprite& sprite)
	{
		m_moveComponentId = MoveComponent::Alloc(GetId(), position, rotation)->GetId();
		m_spriteComponentId = Sprite2DComponent::Alloc(GetId(), sprite)->GetId();
	}

	void RemoveComponents()
	{
		MoveComponent::Free(m_moveComponentId);
		Sprite2DComponent::Free(m_spriteComponentId);
	}

	const u32 GetMoveComponentId() const { return m_moveComponentId; }
	const u32 GetSprite2DComponentId() const { return m_spriteComponentId; }

	MoveComponent* GetMoveComponent() const
	{
		return MoveComponent::GetPool()->Get(m_moveComponentId);
	}

	Sprite2DComponent* GetSpriteComponent() const
	{
		return Sprite2DComponent::GetPool()->Get(m_spriteComponentId);
	}


private:
	Vec2 m_position;
	u32 m_moveComponentId;
	u32 m_spriteComponentId;

	MoveComponent* m_pMoveComponent;
	Sprite2DComponent* m_pSpriteComponent;
};

