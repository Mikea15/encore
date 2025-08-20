
#pragma once

#include "core/core_minimal.h"

#include "memory/base_pool.h"
#include "gfx/types.h"
#include "utils/utils_rand.h"

struct MoveComponent : public PoolId
{
public:
	typedef PoolId Base;

	DECLARE_POOL(MoveComponent);

	MoveComponent() = default;
	MoveComponent(u32 entityId, const Vec2& position, float rotation) 
		: m_entityId(entityId)
		, m_position(position)
		, m_startingPosition(position)
		, m_rotation(rotation)
		, m_accumulatedTime(0.0f)
	{}

	void Update(float deltaTime);

	const Vec2& GetPosition() const { return m_position; }
	const f32 GetRotation() const { return m_rotation; }

	u32 GetEntityId() const { return m_entityId; }

private:
	u32 m_entityId;
	Vec2 m_position;
	Vec2 m_startingPosition;
	f32 m_rotation;
	f32 m_accumulatedTime;
};

