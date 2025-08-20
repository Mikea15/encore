#pragma once

#include "core/core_minimal.h"

#include <GL/glew.h>

class Texture
{
public:
	NO_COPY(Texture);

	// Constructor
	Texture();
	Texture(u32 texId, u32 width, u32 height, u8 channels);

	// Destructor
	~Texture();

	Texture(Texture&& rOther) noexcept;
	Texture& operator=(Texture&& rOther) noexcept;

	void Bind(u32 textureUnit = 0) const;
	void Unbind() const;

	void SetWrapMode(GLenum wrapS, GLenum wrapT);
	void SetFilterMode(GLenum minFilter, GLenum magFilter);

	void GenerateMipmaps();

	u32 GetTextureID() const { return m_textureID; }
	int GetWidth() const { return m_width; }
	int GetHeight() const { return m_height; }
	int GetChannels() const { return m_channels; }
	bool IsValid() const { return m_textureID != 0; }

	// Static utility functions
	static void SetActiveTextureUnit(u32 unit);
	static u32 GetMaxTextureUnits();

private:
	void Cleanup();
	void Move(Texture&& rOther) noexcept;

	u32 m_textureID;
	int m_width;
	int m_height;
	int m_channels;
};
