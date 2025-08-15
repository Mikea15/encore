
#pragma once

#include "core/core_minimal.h"

#include "renderer/sprite.h"
#include "components/move_component.h"
#include "memory/base_pool.h"
#include "utils/utils_rand.h"

struct Sprite2DComponent : public PoolId
{
public:
	typedef PoolId Base;

	DECLARE_POOL(Sprite2DComponent);

	Sprite2DComponent() = default;
	Sprite2DComponent(u32 entityId, const Sprite& sprite)
		: m_entityId(entityId)
		, m_sprite(sprite)
	{}

	const Sprite& GetSprite() const { return m_sprite; }
	u32 GetEntityId() const { return m_entityId; }

private:
	u32 m_entityId;
	Sprite m_sprite;
};

