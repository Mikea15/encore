#include "texture.h"

IMPLEMENT_POOL(Texture, 40);

Texture::Texture()
	: m_textureID(0)
	, m_width(0)
	, m_height(0)
	, m_format(GL_RGBA)
	, m_internalFormat(GL_RGBA)
{}

Texture::Texture(u32 texId, u32 width, u32 height, GLenum format, GLenum internalFormat)
	: m_textureID(texId)
	, m_width(width)
	, m_height(height)
	, m_format(format)
	, m_internalFormat(internalFormat)
{}

Texture::~Texture()
{
	Cleanup();
}

Texture::Texture(Texture&& rOther) noexcept
	: m_textureID(0)
	, m_width(0)
	, m_height(0)
	, m_format(GL_RGBA)
	, m_internalFormat(GL_RGBA)
{
	Move(std::move(rOther));
}

Texture& Texture::operator=(Texture&& rOther) noexcept
{
	if(this != &rOther)
	{
		Cleanup();
		Move(std::move(rOther));
	}
	return *this;
}

bool Texture::LoadFromFile(const char* rFilePath)
{
	// Note: This is a simplified example. In practice, you'd use a library like stb_image
	// to load actual image files. For now, this creates a placeholder texture.

	// Generate texture ID if not already created
	if(m_textureID == 0)
	{
		glGenTextures(1, &m_textureID);
	}

	// For demonstration, create a simple 2x2 checkerboard pattern
	unsigned char data[] = {
		255, 0, 0, 255,     // Red
		0, 255, 0, 255,     // Green
		0, 0, 255, 255,     // Blue
		255, 255, 0, 255    // Yellow
	};

	glBindTexture(GL_TEXTURE_2D, m_textureID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	// Set default parameters
	SetWrapMode(GL_REPEAT, GL_REPEAT);
	SetFilterMode(GL_LINEAR, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);

	m_width = 2;
	m_height = 2;
	m_format = GL_RGBA;
	m_internalFormat = GL_RGBA;

	return true;
}

void Texture::Bind(u32 textureUnit) const
{
	if(m_textureID != 0)
	{
		AssertMsg(GL_TEXTURE0 + textureUnit <= GL_TEXTURE31, "Over Texture limit 32");

		glActiveTexture(GL_TEXTURE0 + textureUnit);
		glBindTexture(GL_TEXTURE_2D, m_textureID);
	}
}

void Texture::Unbind() const
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::SetWrapMode(GLenum wrapS, GLenum wrapT)
{
	if(m_textureID != 0)
	{
		glBindTexture(GL_TEXTURE_2D, m_textureID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void Texture::SetFilterMode(GLenum minFilter, GLenum magFilter)
{
	if(m_textureID != 0)
	{
		glBindTexture(GL_TEXTURE_2D, m_textureID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void Texture::GenerateMipmaps()
{
	if(m_textureID != 0)
	{
		glBindTexture(GL_TEXTURE_2D, m_textureID);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void Texture::UpdateData(const void* pData, int width, int height, GLenum format, GLenum type)
{
	if(m_textureID != 0 && pData != nullptr)
	{
		glBindTexture(GL_TEXTURE_2D, m_textureID);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, type, pData);

		glBindTexture(GL_TEXTURE_2D, 0);

		m_width = width;
		m_height = height;
		m_format = format;
	}
}

void Texture::SetActiveTextureUnit(u32 unit)
{
	glActiveTexture(GL_TEXTURE0 + unit);
}

u32 Texture::GetMaxTextureUnits()
{
	GLint maxUnits;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxUnits);
	return static_cast<u32>(maxUnits);
}

void Texture::Cleanup()
{
	if(m_textureID != 0)
	{
		glDeleteTextures(1, &m_textureID);
		m_textureID = 0;
	}
}

void Texture::Move(Texture&& rOther) noexcept
{
	m_textureID = rOther.m_textureID;
	m_width = rOther.m_width;
	m_height = rOther.m_height;
	m_format = rOther.m_format;
	m_internalFormat = rOther.m_internalFormat;

	rOther.m_textureID = 0;
	rOther.m_width = 0;
	rOther.m_height = 0;
	rOther.m_format = GL_RGBA;
	rOther.m_internalFormat = GL_RGBA;
}
