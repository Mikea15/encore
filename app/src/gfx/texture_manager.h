#pragma once

#include "core/core_minimal.h"
#include "memory/base_pool.h"
#include "manager/base_singleton.h"
#include "texture.h"

class TextureManager
{
	DECLARE_SINGLETON(TextureManager);

public:
	static Texture CreateEmpty(u32 width, u32 height, GLenum format = GL_RGBA);
	static Texture CreateColored(u32 width, u32 height, u8 color[4], GLenum format = GL_RGBA);

private:
	
};
