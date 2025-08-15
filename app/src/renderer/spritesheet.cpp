#include "spritesheet.h"

SpriteSheet::SpriteSheet(const char* pTexturePath, int spriteWidth, int spriteHeight)
	: m_spriteWidth(spriteWidth), m_spriteHeight(spriteHeight)
{
	m_pTexture = std::make_unique<Texture>();
	if(!m_pTexture->LoadFromFile(pTexturePath))
	{
		LOG_ERROR("Failed to load spritesheet: %s");
		return;
	}

	CalculateSpriteDimensions();
}

void SpriteSheet::CalculateSpriteDimensions()
{
	m_spritesPerRow = m_pTexture->GetWidth() / m_spriteWidth;
	m_spritesPerCol = m_pTexture->GetHeight() / m_spriteHeight;
}

SpriteFrame SpriteSheet::GetSprite(int x, int y) const
{
	float u1 = (float)(x * m_spriteWidth) / m_pTexture->GetWidth();
	float v1 = (float)(y * m_spriteHeight) / m_pTexture->GetHeight();
	float u2 = (float)((x + 1) * m_spriteWidth) / m_pTexture->GetWidth();
	float v2 = (float)((y + 1) * m_spriteHeight) / m_pTexture->GetHeight();

	return { u1, v1, u2, v2 };
}

SpriteFrame SpriteSheet::GetSpriteByIndex(int index) const
{
	int x = index % m_spritesPerRow;
	int y = index / m_spritesPerRow;
	return GetSprite(x, y);
}
