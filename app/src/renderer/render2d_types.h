#pragma once

#include "core/core_minimal.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>

typedef glm::vec2 Vec2;
typedef glm::vec3 Vec3;
typedef glm::vec4 Vec4;
typedef glm::mat4 Mat4;

struct SpriteVertex {
	Vec2 position;
	Vec2 texCoords;
	Vec4 color;
	f32 texId;  // Changed from u32 to f32
};

struct Sprite {
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
		: position(pos), size(sz), texId(tex) {
	}
};
