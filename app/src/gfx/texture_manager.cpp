#include "texture_manager.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_LINEAR
#include "stb_image.h"

#include "utils/utils_path.h"

Texture TextureManager::Create(const u8* pData, u32 width, u32 height, u8 channels)
{
	Assert(width > 0 && height > 0);

	u32 texId;
	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);
	
	// Set default parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	const GLenum format = GetTextureFormat(channels);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, pData);

	glBindTexture(GL_TEXTURE_2D, 0);

	return Texture(texId, width, height, channels);
}

Texture TextureManager::LoadFromFile(const char* pFilePath, bool bFlipVertically)
{
	LOG_INFO("Loading: '%s' from '%s'", pFilePath, utils::GetCurrentWorkingDirectory());
	stbi_set_flip_vertically_on_load(bFlipVertically);

	i32 width, height, channels;
	u8* pData = stbi_load(pFilePath, &width, &height, &channels, 0);
	if(!pData)
	{
		LOG_ERROR("Could not load image from: %s with error\n%s", pFilePath, stbi_failure_reason());

		return {};
	}

	// generate texture.
	Texture tex = Create(pData, width, height, channels);

	stbi_image_free(pData);
	return tex;
}

Texture TextureManager::LoadFromMemory(u8* pData, u32 width, u32 height, u8 channels)
{
	if(!pData)
	{
		LOG_ERROR("Could not load image from memory. Invalid pointer to data provided.");
		return {};
	}

	return Create(pData, width, height, channels);
}

GLenum TextureManager::GetTextureFormat(u8 channels)
{
	switch(channels)
	{
	case 1: return GL_RED;
	case 2: return GL_RG;
	case 3: return GL_RGB;
	case 4: return GL_RGBA;
	default:
		LOG_ERROR("Unsupported number of channels: %d", channels);
		return GL_RGB;
	}
}

