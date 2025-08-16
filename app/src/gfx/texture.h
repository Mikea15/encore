#pragma once

#include "core/core_minimal.h"
#include "gfx/types.h"

#include <string>

class Texture
{
public:
	// Constructor
	Texture();

	// Destructor
	~Texture();

	// Copy constructor and assignment operator (deleted to prevent copying)
	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;

	// Move constructor and assignment operator
	Texture(Texture&& rOther) noexcept;
	Texture& operator=(Texture&& rOther) noexcept;

	// Load texture from file
	bool LoadFromFile(const std::string& rFilePath);

	// Create empty texture with specified dimensions
	bool CreateEmpty(int width, int height, GLenum format = GL_RGBA);

	// Bind texture to specified texture unit
	void Bind(unsigned int textureUnit = 0) const;

	// Unbind texture
	void Unbind() const;

	// Set texture parameters
	void SetWrapMode(GLenum wrapS, GLenum wrapT);
	void SetFilterMode(GLenum minFilter, GLenum magFilter);

	// Generate mipmaps
	void GenerateMipmaps();

	// Update texture data
	void UpdateData(const void* pData, int width, int height, GLenum format = GL_RGBA, GLenum type = GL_UNSIGNED_BYTE);

	// Getters
	unsigned int GetTextureID() const { return m_textureID; }
	int GetWidth() const { return m_width; }
	int GetHeight() const { return m_height; }
	GLenum GetFormat() const { return m_format; }
	bool IsValid() const { return m_textureID != 0; }

	// Static utility functions
	static void SetActiveTextureUnit(unsigned int unit);
	static unsigned int GetMaxTextureUnits();

private:
	void Cleanup();
	void Move(Texture&& rOther) noexcept;

	unsigned int m_textureID;
	int m_width;
	int m_height;
	GLenum m_format;
	GLenum m_internalFormat;
};
