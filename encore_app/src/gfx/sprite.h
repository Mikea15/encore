#pragma once

#include "core/core_minimal.h"
#include "types.h"

#include <GL/glew.h>

struct Sprite
{
	Vec2 position = { 0.0f, 0.0f };
	Vec2 size = { 1.0f, 1.0f };
	f32 rotation = 0.0f;
	Vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLuint texId = 0;

	Vec2 uvMin = { 0.0f, 0.0f };
	Vec2 uvMax = { 1.0f, 1.0f };

	// Add constructors for convenience
	Sprite() = default;
	Sprite(Vec2 pos, Vec2 sz, GLuint tex = 0)
		: position(pos), size(sz), texId(tex)
	{}
};
