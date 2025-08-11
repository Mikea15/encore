
#pragma once

#include "core/core_minimal.h"

#include "memory/base_pool.h"
#include "renderer/render2d_types.h"
#include "utils/utils_rand.h"

struct MoveComponent
{
public:
	DECLARE_POOL(MoveComponent);

	MoveComponent() = default;
	MoveComponent(const Vec2& position, float rotation) 
		: m_position(position)
		, m_startingPosition(position)
		, m_rotation(rotation)
		, m_accumulatedTime(0.0f)
	{}

	void Update(float deltaTime);

	const Vec2& GetPosition() const { return m_position; }
	const f32 GetRotation() const { return m_rotation; }

private:
	Vec2 m_position;
	Vec2 m_startingPosition;
	f32 m_rotation;
	f32 m_accumulatedTime;
};

