#include "sprite_sheet.h"

#include <algorithm>

Spritesheet::Spritesheet()
	: m_tileWidth(0)
	  , m_tileHeight(0)
	  , m_columns(0)
	  , m_rows(0)
{
}

Spritesheet::Spritesheet(const Texture& texture, u32 tileWidth, u32 tileHeight)
	: m_texture(texture)
	  , m_tileWidth(tileWidth)
	  , m_tileHeight(tileHeight)
{
	if (texture.GetWidth() > 0 && texture.GetHeight() > 0)
	{
		m_columns = texture.GetWidth() / tileWidth;
		m_rows = texture.GetHeight() / tileHeight;
	}
}

Spritesheet::~Spritesheet()
{
	// Texture cleanup is handled by the Texture class destructor
}

bool Spritesheet::Initialize(const Texture& texture, u32 tileWidth, u32 tileHeight)
{
	if (texture.GetWidth() == 0 || texture.GetHeight() == 0 || tileWidth == 0 || tileHeight == 0)
	{
		LOG_ERROR("Invalid texture or tile dimensions");
		return false;
	}

	m_texture = texture;
	m_tileWidth = tileWidth;
	m_tileHeight = tileHeight;
	m_columns = texture.GetWidth() / tileWidth;
	m_rows = texture.GetHeight() / tileHeight;

	LOG_INFO("Initialized spritesheet: %dx%d tiles (%dx%d pixels each)",
	         m_columns, m_rows, m_tileWidth, m_tileHeight);

	return true;
}

SpriteFrame Spritesheet::GetTile(u32 column, u32 row) const
{
	if (column >= m_columns || row >= m_rows)
	{
		LOG_ERROR("Tile coordinates out of bounds: (%d,%d) max: (%d,%d)",
		          column, row, m_columns-1, m_rows-1);
		return SpriteFrame(0.0f, 0.0f, 1.0f, 1.0f);
	}

	float u1, v1, u2, v2;
	CalculateUV(column, row, u1, v1, u2, v2);
	return SpriteFrame(u1, v1, u2, v2);
}

void Spritesheet::AddAnimation(const std::string& name, const std::vector<u32>& frameIndices,
                               float frameDuration, bool loop)
{
	Animation animation(name, loop);

	for (u32 index : frameIndices)
	{
		if (index < m_columns * m_rows)
		{
			SpriteFrame frame = GetFrameFromIndex(index);
			frame.duration = frameDuration;
			animation.frames.push_back(frame);
		}
		else
		{
			LOG_ERROR("Frame index %d out of bounds for animation '%s'", index, name.c_str());
		}
	}

	if (!animation.frames.empty())
	{
		m_animations.push_back(animation);
		LOG_INFO("Added animation '%s' with %zu frames", name.c_str(), animation.frames.size());
	}
}

void Spritesheet::AddAnimation(const std::string& name, u32 startCol, u32 startRow,
                               u32 frameCount, bool horizontal, float frameDuration, bool loop)
{
	std::vector<u32> frameIndices;

	if (horizontal)
	{
		// Add frames horizontally (left to right)
		for (u32 i = 0; i < frameCount; ++i)
		{
			u32 col = startCol + i;
			u32 row = startRow;

			// Wrap to next row if needed
			if (col >= m_columns)
			{
				col = col % m_columns;
				row = startRow + (startCol + i) / m_columns;
			}

			if (row < m_rows)
			{
				frameIndices.push_back(GetTileIndex(col, row));
			}
		}
	}
	else
	{
		// Add frames vertically (top to bottom)
		for (u32 i = 0; i < frameCount; ++i)
		{
			u32 col = startCol;
			u32 row = startRow + i;

			// Wrap to next column if needed
			if (row >= m_rows)
			{
				row = row % m_rows;
				col = startCol + (startRow + i) / m_rows;
			}

			if (col < m_columns)
			{
				frameIndices.push_back(GetTileIndex(col, row));
			}
		}
	}

	AddAnimation(name, frameIndices, frameDuration, loop);
}

const Animation* Spritesheet::GetAnimation(const std::string& name) const
{
	auto it = std::find_if(m_animations.begin(), m_animations.end(),
	                       [&name](const Animation& anim) { return anim.name == name; });

	return (it != m_animations.end()) ? &(*it) : nullptr;
}

u32 Spritesheet::GetTileIndex(u32 column, u32 row) const
{
	return row * m_columns + column;
}

SpriteFrame Spritesheet::GetFrameFromIndex(u32 index) const
{
	u32 column = index % m_columns;
	u32 row = index / m_columns;
	return GetTile(column, row);
}

void Spritesheet::CalculateUV(u32 column, u32 row, float& u1, float& v1, float& u2, float& v2) const
{
	float texWidth = static_cast<float>(m_texture.GetWidth());
	float texHeight = static_cast<float>(m_texture.GetHeight());

	u1 = (column * m_tileWidth) / texWidth;
	v1 = (row * m_tileHeight) / texHeight;
	u2 = ((column + 1) * m_tileWidth) / texWidth;
	v2 = ((row + 1) * m_tileHeight) / texHeight;
}
