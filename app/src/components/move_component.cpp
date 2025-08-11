
#include "move_component.h"

IMPLEMENT_POOL(MoveComponent, 100000);

void MoveComponent::Update(float deltaTime)
{
	m_accumulatedTime += deltaTime * utils::GetFloat(2.0f, 7.0f);
	m_rotation += deltaTime * rand() * 0.03f;

	m_position = Vec2(sin(m_accumulatedTime), cos(m_accumulatedTime)) * 15.0f + m_startingPosition;
}
