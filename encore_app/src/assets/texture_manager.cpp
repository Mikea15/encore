#include "texture_manager.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_LINEAR
#include "stb_image.h"

#include "utils/utils_path.h"

GLuint TextureManager::CreateTexture(const u8* pData, u32 width, u32 height, u8 channels)
{
	Assert(width > 0 && height > 0);
	Assert(pData != nullptr);

	u32 texId;
	glGenTextures(1, &texId);

	if(texId == 0)
	{
		LOG_ERROR("Failed to generate texture ID");
		return 0;
	}

	LOG_INFO("Generated texture ID: %d for %dx%d image with %d channels", texId, width, height, channels);

	glBindTexture(GL_TEXTURE_2D, texId);

	GLenum error = glGetError();
	if(error != GL_NO_ERROR)
	{
		LOG_ERROR("Error binding texture %d: 0x%X", texId, error);
		glDeleteTextures(1, &texId);
		return 0;
	}

	// Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	const GLenum dataFormat = GetTextureFormat(channels);

	GLenum internalFormat;
	switch(channels)
	{
	case 1: internalFormat = GL_R8; break;
	case 2: internalFormat = GL_RG8; break;
	case 3: internalFormat = GL_RGB8; break;
	case 4: internalFormat = GL_RGBA8; break;
	default:
		LOG_ERROR("Unsupported channel count: %d", channels);
		glDeleteTextures(1, &texId);
		return 0;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, pData);

	error = glGetError();
	if(error != GL_NO_ERROR)
	{
		LOG_ERROR("Error uploading texture data for ID %d: 0x%X", texId, error);
		glDeleteTextures(1, &texId);
		return 0;
	}

	// Verify texture was created properly
	GLint actualWidth;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &actualWidth);

	if(actualWidth != static_cast<GLint>(width))
	{
		LOG_ERROR("Texture creation failed - width mismatch");
		glDeleteTextures(1, &texId);
		return 0;
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	// Store texture info
	m_pTextures[texId] = { width, height, channels, "manual_create" };

	LOG_INFO("Successfully created texture %d", texId);
	return texId;
}

GLuint TextureManager::LoadFromFile(const char* pFilePath, bool bFlipVertically)
{
	LOG_INFO("Loading texture from: '%s'", pFilePath);

	stbi_set_flip_vertically_on_load(bFlipVertically);

	i32 width, height, channels;
	u8* pData = stbi_load(pFilePath, &width, &height, &channels, 0);

	if(!pData)
	{
		LOG_ERROR("stbi_load failed for '%s': %s", pFilePath, stbi_failure_reason());
		return 0;
	}

	LOG_INFO("Loaded image: %dx%d with %d channels", width, height, channels);

	GLuint textureId = CreateTexture(pData, static_cast<u32>(width), static_cast<u32>(height), static_cast<u8>(channels));

	stbi_image_free(pData);

	if(textureId != 0)
	{
		// Update file path for debugging
		m_pTextures[textureId].m_filePath = pFilePath;
		LOG_INFO("Successfully loaded texture %d from '%s'", textureId, pFilePath);
	}

	return textureId;
}

Spritesheet TextureManager::LoadSpritesheet(const char* pFilePath, u32 tileWidth, u32 tileHeight, bool bFlipVertically)
{
	GLuint textureId = LoadFromFile(pFilePath, bFlipVertically);
	if (textureId == 0)
	{
		LOG_ERROR("Failed to load texture for spritesheet: '%s'", pFilePath);
		return Spritesheet();
	}

	// Get texture info
	u32 width, height;
	u8 channels;
	if (!GetTextureInfo(textureId, width, height, channels))
	{
		LOG_ERROR("Failed to get texture info for spritesheet");
		return Spritesheet();
	}

	// Create texture object
	Texture texture(textureId, width, height, channels);

	// Create and return spritesheet
	Spritesheet spritesheet(texture, tileWidth, tileHeight);

	LOG_INFO("Created spritesheet from '%s': %dx%d tiles", pFilePath,
			 spritesheet.GetColumns(), spritesheet.GetRows());

	return spritesheet;
}

Spritesheet TextureManager::CreateSpritesheet(GLuint textureId, u32 tileWidth, u32 tileHeight)
{
	if (!IsValidTexture(textureId))
	{
		LOG_ERROR("Invalid texture ID: %d", textureId);
		return Spritesheet();
	}

	u32 width, height;
	u8 channels;
	if (!GetTextureInfo(textureId, width, height, channels))
	{
		LOG_ERROR("Failed to get texture info for spritesheet");
		return Spritesheet();
	}

	Texture texture(textureId, width, height, channels);
	return Spritesheet(texture, tileWidth, tileHeight);
}

void TextureManager::DeleteTexture(GLuint textureId)
{
	auto it = m_pTextures.find(textureId);
	if(it != m_pTextures.end())
	{
		glDeleteTextures(1, &textureId);
		m_pTextures.erase(it);
		LOG_INFO("Deleted texture %d", textureId);
	}
}

void TextureManager::DeleteAllTextures()
{
	for(const auto& pair : m_pTextures)
	{
		glDeleteTextures(1, &pair.first);
	}
	m_pTextures.clear();
	LOG_INFO("Deleted all textures");
}

bool TextureManager::IsValidTexture(GLuint textureId) const
{
	return m_pTextures.find(textureId) != m_pTextures.end() && glIsTexture(textureId);
}

bool TextureManager::GetTextureInfo(GLuint textureId, u32& width, u32& height, u8& channels)
{
	auto it = m_pTextures.find(textureId);
	if(it != m_pTextures.end())
	{
		width = it->second.m_width;
		height = it->second.m_height;
		channels = it->second.m_channels;
		return true;
	}
	return false;
}

GLenum TextureManager::GetTextureFormat(u8 channels)
{
	switch(channels)
	{
	case 1: return GL_RED;     // Grayscale
	case 2: return GL_RG;      // Grayscale + Alpha
	case 3: return GL_RGB;     // RGB
	case 4: return GL_RGBA;    // RGBA
	default:
		LOG_ERROR("Unsupported channel count: %d", channels);
		return GL_RGB; // fallback
	}
}
