#pragma once

#include "core/core_minimal.h"
#include <GL/glew.h>

class Texture
{
public:

	bool LoadFromFile(const char* pFilePath);
	void Bind(unsigned int slot = 0) const;
	void Unbind() const;

	int GetWidth() const { return m_width; }
	int GetHeight() const { return m_height; }
	GLuint GetID() const { return m_textureID; }

private:
	GLuint m_textureID;
	int m_width;
	int m_height;
	int m_channels;
};
