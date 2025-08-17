#include "texture_manager.h"

Texture TextureManager::CreateEmpty(u32 width, u32 height, GLenum format /*= GL_RGBA*/)
{
	return CreateColored(width, height, nullptr, format);
}

Texture TextureManager::CreateColored(u32 width, u32 height, u8* color, GLenum format)
{
	Assert(width > 0 && height > 0);

	u32 texId;
	glGenTextures(1, &texId);

	glBindTexture(GL_TEXTURE_2D, texId);
	
	// Determine internal format based on format
	GLenum internalFormat = format;
	if(format == GL_RGB) internalFormat = GL_RGB8;
	else if(format == GL_RGBA) internalFormat = GL_RGBA8;

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, color);

	// Set default parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);

	return Texture(texId, width, height, format, internalFormat);
}
