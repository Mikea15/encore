
#pragma once

#include "assets/animated_sprite.h"
#include "core/core_minimal.h"

#include "memory/base_pool.h"
#include "utils/utils_rand.h"
#include "gfx/sprite.h"

struct AnimatedSpriteComponent : public PoolId
{
public:
	typedef PoolId Base;

	DECLARE_POOL(AnimatedSpriteComponent);

	AnimatedSpriteComponent() = default;
	AnimatedSpriteComponent(u32 entityId, const AnimatedSprite& sprite)
		: m_entityId(entityId)
		, m_sprite(sprite)
	{}

	const AnimatedSprite& GetSprite() const { return m_sprite; }
	u32 GetEntityId() const { return m_entityId; }

private:
	u32 m_entityId;
	AnimatedSprite m_sprite;
};

