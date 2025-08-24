#include "sprite_sheet.h"

#include <algorithm>

Spritesheet::Spritesheet()
	: m_tileWidth(0)
	, m_tileHeight(0)
	, m_columns(0)
	, m_rows(0)
	, m_textureWidth(0.0f)
	, m_textureHeight(0.0f)
{ }

Spritesheet::Spritesheet(GLuint textureId, u32 tileWidth, u32 tileHeight, f32 textureWidth, f32 textureHeight)
	: m_texture(textureId)
	, m_tileWidth(tileWidth)
	, m_tileHeight(tileHeight)
	, m_textureWidth(textureWidth)
	, m_textureHeight(textureHeight)
{
	Assert(tileWidth > 0 && tileHeight > 0);
	Assert(textureWidth > 0.0f && textureHeight > 0.0f);

	m_columns = static_cast<u32>(textureWidth / (f32)tileWidth);
	m_rows = static_cast<u32>(textureHeight / (f32)tileHeight);
}

bool Spritesheet::Initialize(GLuint textureId, u32 tileWidth, u32 tileHeight, f32 textureWidth, f32 textureHeight)
{
	Assert(tileWidth > 0 && tileHeight > 0);

	Assert(tileWidth > 0 && tileHeight > 0);
	Assert(textureWidth > 0.0f && textureHeight > 0.0f);

	m_texture = textureId;
	m_tileWidth = tileWidth;
	m_tileHeight = tileHeight;
	m_textureWidth = textureWidth;
	m_textureHeight = textureHeight;
	m_columns = static_cast<u32>(textureWidth / (f32)tileWidth);
	m_rows = static_cast<u32>(textureHeight / (f32)tileHeight);

	LOG_INFO("Initialized spritesheet: %dx%d tiles (%dx%d pixels each)", m_columns, m_rows, m_tileWidth, m_tileHeight);
	return true;
}

SpriteFrame Spritesheet::GetTile(u32 column, u32 row) const
{
	if (column >= m_columns || row >= m_rows)
	{
		LOG_ERROR("Tile coordinates out of bounds: (%d,%d) max: (%d,%d)", column, row, m_columns-1, m_rows-1);
		return SpriteFrame(0.0f, 0.0f, 1.0f, 1.0f);
	}

	float u1, v1, u2, v2;
	CalculateUV(column, row, u1, v1, u2, v2);
	return SpriteFrame(u1, v1, u2, v2);
}

void Spritesheet::AddAnimation(const std::string& name, const std::vector<u32>& frameIndices, float frameDuration,
                               bool loop)
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

void Spritesheet::Bind(u32 textureUnit) const
{
	glBindTexture(GL_TEXTURE_2D + textureUnit, m_texture);
}

void Spritesheet::Unbind() const
{
	glBindTexture(GL_TEXTURE_2D, 0);
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
	u1 = static_cast<f32>(column * m_tileWidth) / m_textureWidth;
	u2 = static_cast<f32>((column + 1) * m_tileWidth) / m_textureWidth;
	v1 = static_cast<f32>(row * m_tileHeight) / m_textureHeight;
	v2 = static_cast<f32>((row + 1) * m_tileHeight) / m_textureHeight;
}
