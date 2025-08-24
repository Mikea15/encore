#pragma once

#include "core/core_types.h"

#include "camera_2d.h"
#include "entity/entity.h"
#include "shader.h"
#include "types.h"

#include <vector>
#include <GL/glew.h>

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
	void DrawSprite(Vec2 position, Vec2 size, GLuint texture = 0, Vec4 color = Vec4(1));
	// PERF: Move Semantics Here?
	void DrawRotatedSprite(Vec2 position, Vec2 size, f32 rotation, GLuint texture = 0, Vec4 color = Vec4(1));

	void DrawSprite(const Vec2& position, float rotation, const SpriteFrame& frame, GLuint textureId,
				const Vec2& size, const Vec4& color = Vec4(1.0f));

	void DrawTile(const Vec2& position, const Spritesheet& spritesheet, u32 column, u32 row,
				  const Vec2& size, const Vec4& color = Vec4(1.0f));

	void DrawAnimatedSprite(const Vec2& position, float rotation, const AnimatedSprite& animatedSprite,
						   const Vec2& size, const Vec4& color = Vec4(1.0f));

	// Convenience overloads
	void DrawTile(const Vec2& position, const Spritesheet& spritesheet, u32 tileIndex,
				  const Vec2& size, const Vec4& color = Vec4(1.0f));

	void End();

	const Stats& GetStats() const { return m_renderStats; }

private:
	void CreateBuffers();
	void CreateShader();

	f32 GetTextureIndex(GLuint texId);

	void Flush();

	static constexpr u32 MAX_SPRITES = 100000;
	static constexpr u32 MAX_VERTICES = MAX_SPRITES * 4;
	static constexpr u32 MAX_INDICES = MAX_SPRITES * 6;
	static constexpr u8 MAX_TEXTURES = 32; // Max texture slots

	// OpenGL objects
	GLuint m_VAO = 0, m_VBO = 0, m_EBO = 0;

	GLuint m_whiteTextureId;
	Shader m_shader;

	// Batch data
	std::vector<SpriteVertex> m_vertices;
	std::vector<GLuint> m_textureSlots;
	u32 m_currentTextureSlot = 0;

	// Statistics
	Stats m_renderStats;
};
