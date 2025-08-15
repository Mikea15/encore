#pragma once

#include "core/core_types.h"

#include "render2d_types.h"
#include "render2d_camera.h"
#include "renderer_shader.h"

#include <GL/glew.h>

#include <string>
#include <vector>
#include "entity/entity.h"
#include "spritesheet.h"

struct Sprite;

class SpriteBatchRenderer
{
public:
	struct Stats
	{
		u32 drawCalls = 0;
		u32 spritesDrawn = 0;
		u32 verticesDrawn = 0;

		void Reset()
		{
			drawCalls = 0;
			spritesDrawn = 0;
			verticesDrawn = 0;
		}
	};

	SpriteBatchRenderer() = default;

	void Init();
	void Clear();

	void Begin(const Camera2D& cam, f32 viewportWidth, f32 viewportHeight);
	void DrawSprite(const Vec2& position, float rotation, const Sprite& sprite);
	// PERF: Move Semantics Here?
	void DrawSprite(Vec2 position, Vec2 size, GLuint texture = 0, Vec4 color = Vec4(1, 1, 1, 1));
	// PERF: Move Semantics Here?
	void DrawRotatedSprite(Vec2 position, Vec2 size, f32 rotation, GLuint texture = 0, Vec4 color = Vec4(1, 1, 1, 1));

	void End();

	const Stats& GetStats() const { return stats; }

private:
	void CreateBuffers();

	void CreateShader();

	void CreateWhiteTexture();

	f32 GetTextureIndex(GLuint texId);

	void Flush();

	static constexpr u32 MAX_SPRITES = 100000;
	static constexpr u32 MAX_VERTICES = MAX_SPRITES * 4;
	static constexpr u32 MAX_INDICES = MAX_SPRITES * 6;
	static constexpr u8 MAX_TEXTURES = 32; // Max texture slots

	// OpenGL objects
	GLuint VAO = 0, VBO = 0, EBO = 0;
	GLuint whiteTexture = 0;
	Shader shader;

	SpriteSheet sprite;

	// Batch data
	std::vector<SpriteVertex> vertices;
	std::vector<GLuint> textureSlots;
	u32 currentTextureSlot = 0;

	// Statistics
	Stats stats;
};
