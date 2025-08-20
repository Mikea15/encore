#pragma once

#include "core/core_minimal.h"

#include <GL/glew.h>

class Texture
{
public:
	// Constructor
	Texture();
	Texture(GLuint texId, u32 width, u32 height, u8 channels);

	// ~Texture() = delete; 

	Texture(Texture&& rOther) noexcept;
	Texture& operator=(Texture&& rOther) noexcept;

	void Bind(u32 textureUnit = 0) const;
	void Unbind() const;

	void SetWrapMode(GLenum wrapS, GLenum wrapT);
	void SetFilterMode(GLenum minFilter, GLenum magFilter);

	void GenerateMipmaps();

	GLuint GetTextureID() const { return m_textureID; }
	int GetWidth() const { return m_width; }
	int GetHeight() const { return m_height; }
	int GetChannels() const { return m_channels; }
	bool IsValid() const { return m_textureID != 0; }

	// Static utility functions
	static void SetActiveTextureUnit(u32 unit);
	static u32 GetMaxTextureUnits();

private:
	GLuint m_textureID;
	int m_width;
	int m_height;
	int m_channels;
};
