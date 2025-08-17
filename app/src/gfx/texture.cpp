#include "texture.h"

// IMPLEMENT_POOL(Texture, 40);

Texture::Texture()
	: m_textureID(0)
	, m_width(0)
	, m_height(0)
	, m_channels(0)
{}

Texture::Texture(u32 texId, u32 width, u32 height, u8 channels)
	: m_textureID(texId)
	, m_width(width)
	, m_height(height)
	, m_channels(channels)
{}

Texture::~Texture()
{
	Cleanup();
}

Texture::Texture(Texture&& rOther) noexcept
	: m_textureID(0)
	, m_width(0)
	, m_height(0)
	, m_channels(0)
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
	m_channels = rOther.m_channels;

	rOther.m_textureID = 0;
	rOther.m_width = 0;
	rOther.m_height = 0;
	rOther.m_channels = 0;
}
