#pragma once

#include "core/core_minimal.h"

#include <string>
#include <vector>
#include <GL/glew.h>

struct SpriteFrame
{
	f32 u1, v1; // Top-left UV coordinates
	f32 u2, v2; // Bottom-right UV coordinates
	f32 duration; // Duration in seconds (for animation)

	SpriteFrame() = default;
	SpriteFrame(float _u1, float _v1, float _u2, float _v2, float _duration = 0.1f)
		: u1(_u1), v1(_v1), u2(_u2), v2(_v2), duration(_duration)
	{ }

};

struct Animation
{
	std::vector<SpriteFrame> frames;
	bool loop;
	std::string name;

	Animation(const std::string& animName = "", bool shouldLoop = true)
		: name(animName), loop(shouldLoop)
	{
	}
};

class Spritesheet
{
public:
	Spritesheet();
	Spritesheet(GLuint textureId, u32 tileWidth, u32 tileHeight, f32 textureWidth, f32 textureHeight);

	bool Initialize(GLuint textureId, u32 tileWidth, u32 tileHeight, f32 textureWidth, f32 textureHeight);

	SpriteFrame GetTile(u32 column, u32 row) const;

	void AddAnimation(const std::string& name, const std::vector<u32>& frameIndices,
	                  float frameDuration = 0.1f, bool loop = true);
	void AddAnimation(const std::string& name, u32 startCol, u32 startRow,
	                  u32 frameCount, bool horizontal = true, float frameDuration = 0.1f, bool loop = true);

	// Get animation by name
	const Animation* GetAnimation(const std::string& name) const;

	u32 GetColumns() const { return m_columns; }
	u32 GetRows() const { return m_rows; }
	u32 GetTileWidth() const { return m_tileWidth; }
	u32 GetTileHeight() const { return m_tileHeight; }

	GLuint GetTextureId() const { return m_texture; }
	void Bind(u32 textureUnit = 0) const;
	void Unbind() const;

	// Convert 2D coordinates to 1D index
	u32 GetTileIndex(u32 column, u32 row) const;

	// Convert 1D index to UV coordinates
	SpriteFrame GetFrameFromIndex(u32 index) const;

private:
	GLuint m_texture;
	u32 m_tileWidth;
	u32 m_tileHeight;
	u32 m_columns;
	u32 m_rows;
	f32 m_textureWidth;
	f32 m_textureHeight;

	std::vector<Animation> m_animations;

	// Calculate UV coordinates for a tile
	void CalculateUV(u32 column, u32 row, float& u1, float& v1, float& u2, float& v2) const;
};
