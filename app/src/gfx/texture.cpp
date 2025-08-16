#include "texture.h"

Texture::Texture()
	: m_textureID(0)
	, m_width(0)
	, m_height(0)
	, m_format(GL_RGBA)
	, m_internalFormat(GL_RGBA)
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

bool Texture::LoadFromFile(const std::string& rFilePath)
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

	m_width = 2;
	m_height = 2;
	m_format = GL_RGBA;
	m_internalFormat = GL_RGBA;

	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}

bool Texture::CreateEmpty(int width, int height, GLenum format)
{
	if(width <= 0 || height <= 0)
		return false;

	// Generate texture ID if not already created
	if(m_textureID == 0)
	{
		glGenTextures(1, &m_textureID);
	}

	glBindTexture(GL_TEXTURE_2D, m_textureID);

	// Determine internal format based on format
	GLenum internalFormat = format;
	if(format == GL_RGB) internalFormat = GL_RGB8;
	else if(format == GL_RGBA) internalFormat = GL_RGBA8;

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);

	// Set default parameters
	SetWrapMode(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	SetFilterMode(GL_LINEAR, GL_LINEAR);

	m_width = width;
	m_height = height;
	m_format = format;
	m_internalFormat = internalFormat;

	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}

void Texture::Bind(unsigned int textureUnit) const
{
	if(m_textureID != 0)
	{
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

void Texture::SetActiveTextureUnit(unsigned int unit)
{
	glActiveTexture(GL_TEXTURE0 + unit);
}

unsigned int Texture::GetMaxTextureUnits()
{
	GLint maxUnits;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxUnits);
	return static_cast<unsigned int>(maxUnits);
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
