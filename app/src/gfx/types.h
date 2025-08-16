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
	f32 texId;
};
