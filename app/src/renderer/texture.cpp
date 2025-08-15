
#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

bool Texture::LoadFromFile(const char* pFilePath)
{
	unsigned char* pData = stbi_load(pFilePath, &m_width, &m_height, &m_channels, 0);

	if(!pData)
	{
		LOG_ERROR("Failed to load texture: %s", pFilePath);
		return false;
	}

	glGenTextures(1, &m_textureID);
	glBindTexture(GL_TEXTURE_2D, m_textureID);

	// Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // Pixel art friendly
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	GLenum format = (m_channels == 4) ? GL_RGBA : GL_RGB;
	glTexImage2D(GL_TEXTURE_2D, 0, format, m_width, m_height, 0, format, GL_UNSIGNED_BYTE, pData);

	stbi_image_free(pData);
	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}
