#pragma once

#include "core/core_minimal.h"
#include "texture.h"

#include <memory>
#include <string>

struct SpriteFrame
{
	f32 u1, v1; // Top-left UV coordinates
	f32 u2, v2; // Bottom-right UV coordinates
};

class SpriteSheet
{
public:
	SpriteSheet(const char* pTexturePath, int spriteWidth, int spriteHeight);

	SpriteFrame GetSprite(int x, int y) const;
	SpriteFrame GetSpriteByIndex(int index) const;

	void Bind() const { m_pTexture->Bind(); }
	i32 GetSpriteCount() const { return m_spritesPerRow * m_spritesPerCol; }

private:
	void CalculateSpriteDimensions();

	std::unique_ptr<Texture> m_pTexture;
	u16 m_spriteWidth;
	u16 m_spriteHeight;
	u16 m_spritesPerRow;
	u16 m_spritesPerCol;
};
