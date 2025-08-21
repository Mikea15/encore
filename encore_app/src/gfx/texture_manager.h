#pragma once

#include "core/core_minimal.h"

#include "manager/base_singleton.h"
#include "texture.h"

#include <GL/glew.h>
#include <unordered_map>
#include <string>

class TextureManager
{
	DECLARE_SINGLETON(TextureManager);

public:
	// Return texture ID instead of Texture object
	GLuint CreateTexture(const u8* pData, u32 width, u32 height, u8 channels);
	GLuint LoadFromFile(const char* pFilePath, bool bFlipVertically = false);

	// Get texture properties
	bool GetTextureInfo(GLuint textureId, u32& width, u32& height, u8& channels);

	// Manual texture management
	void DeleteTexture(GLuint textureId);
	void DeleteAllTextures();

	// Validation
	bool IsValidTexture(GLuint textureId) const;
	
	struct GPUMemStats
	{
		GLint total;
		GLint available;
	};

	GPUMemStats GetGPUMemoryUsage() const
	{
#ifdef GL_NVX_gpu_memory_info
		GPUMemStats stats;
		GLint totalMemory, availableMemory;
		glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &stats.total);
		glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &stats.available);
		return stats;
#endif
	}

private:
	struct TextureData
	{
		u32 m_width;
		u32 m_height;
		u8 m_channels;
		std::string m_filePath; // For debugging
	};
	GLenum GetTextureFormat(u8 channels);

	std::unordered_map<GLuint, TextureData> m_pTextures;
};
