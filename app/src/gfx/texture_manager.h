#pragma once

#include "core/core_minimal.h"
#include "memory/base_pool.h"
#include "manager/base_singleton.h"
#include "texture.h"

class TextureManager
{
	DECLARE_SINGLETON(TextureManager);

public:
	Texture Create(const u8* pData, u32 width, u32 height, u8 channels);

	Texture LoadFromFile(const char* pFilePath, bool bFlipVertically = true);
	Texture LoadFromMemory(u8* pData, u32 width, u32 height, u8 channels);

private:
	GLenum GetTextureFormat(u8 channels);
};
