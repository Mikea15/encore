#include "sprite_renderer.h"

#include "texture.h"
#include "texture_manager.h"

void SpriteBatchRenderer::Init()
{
	CreateBuffers();
	CreateShader();

	m_vertices.reserve(MAX_VERTICES);
	m_textureSlots.reserve(MAX_TEXTURES);

	u8 white[4] = { 255, 255, 255, 255 };
	m_whiteTexture = TextureManager::GetInstance().Create(white, 1, 1, 4);
}

void SpriteBatchRenderer::Clear()
{
	m_shader.Clear();

	if(m_VAO) { glDeleteVertexArrays(1, &m_VAO); }
	if(m_VBO) { glDeleteBuffers(1, &m_VBO); }
	if(m_EBO) { glDeleteBuffers(1, &m_EBO); }
}

void SpriteBatchRenderer::Begin(const Camera2D& cam, f32 viewportWidth, f32 viewportHeight)
{
	m_renderStats.Reset();
	m_vertices.clear();
	m_textureSlots.clear();
	m_currentTextureSlot = 0;

	// Always have white texture in slot 0
	m_textureSlots.push_back(m_whiteTexture.GetTextureID());
	m_currentTextureSlot = 1;

	// Set up camera matrices
	m_shader.Use();
	const Mat4 view = cam.GetViewMatrix();
	const Mat4 projection = cam.GetProjectionMatrix(viewportWidth, viewportHeight);
	const Mat4 viewProjection = projection * view;

	m_shader.SetMat4("uViewProjection", viewProjection);

	// Set up texture samplers
	i32 samplers[MAX_TEXTURES];
	for(i32 i = 0; i < MAX_TEXTURES; ++i)
	{
		samplers[i] = i;
	}
	m_shader.SetIntArray("uTextures", samplers, MAX_TEXTURES);
}

void SpriteBatchRenderer::DrawSprite(const Vec2& position, float rotation, const Sprite& sprite)
{
	f32 textureIndex = GetTextureIndex(sprite.texId);

	// Check if we need to flush (too many sprites or textures)
	if(m_vertices.size() + 4 > MAX_VERTICES ||
		(textureIndex == -1.0f && m_currentTextureSlot >= MAX_TEXTURES))
	{
		Flush();

		// Restart batch - but don't call Begin() again, just reset internal state
		m_vertices.clear();
		m_textureSlots.clear();
		m_textureSlots.push_back(m_whiteTexture.GetTextureID());
		m_currentTextureSlot = 1;

		textureIndex = GetTextureIndex(sprite.texId);
	}

	// Create quad m_vertices
	Vec2 halfSize = sprite.size * 0.5f;

	// Calculate corner positions
	Vec2 corners[4] = {
		{ -halfSize.x, -halfSize.y }, // Bottom-left
		{ halfSize.x, -halfSize.y }, // Bottom-right
		{ halfSize.x,  halfSize.y }, // Top-right
		{ -halfSize.x,  halfSize.y }  // Top-left
	};

	// Apply rotation if needed
	if(rotation != 0.0f)
	{
		f32 rCos = cos(glm::radians(rotation));
		f32 rSin = sin(glm::radians(rotation));

		for(i8 i = 0; i < 4; ++i)
		{
			f32 x = corners[i].x;
			f32 y = corners[i].y;
			corners[i].x = x * rCos - y * rSin;
			corners[i].y = x * rSin + y * rCos;
		}
	}

	// Translate to world position
	for(i8 i = 0; i < 4; ++i)
	{
		corners[i] += position;
	}

	// UV coordinates
	Vec2 uvs[4] = {
		{ sprite.uvMin.x, sprite.uvMin.y }, // Bottom-left
		{ sprite.uvMax.x, sprite.uvMin.y }, // Bottom-right
		{ sprite.uvMax.x, sprite.uvMax.y }, // Top-right
		{ sprite.uvMin.x, sprite.uvMax.y }  // Top-left
	};

	// Add m_vertices to batch
	for(i8 i = 0; i < 4; ++i)
	{
		m_vertices.emplace_back(SpriteVertex{ corners[i], uvs[i], sprite.color, textureIndex });
	}

	m_renderStats.spritesDrawn++;
}

// PERF: Move Semantics Here?
void SpriteBatchRenderer::DrawSprite(Vec2 position, Vec2 size, GLuint texture, Vec4 color)
{
	Sprite sprite(position, size, texture);
	sprite.color = color;
	DrawSprite(position, sprite.rotation, sprite);
}

// PERF: Move Semantics Here?
void SpriteBatchRenderer::DrawRotatedSprite(Vec2 position, Vec2 size, f32 rotation, GLuint texture, Vec4 color)
{
	Sprite sprite(position, size, texture);
	sprite.rotation = rotation;
	sprite.color = color;
	DrawSprite(position, sprite.rotation, sprite);
}

void SpriteBatchRenderer::End()
{
	Flush();
}

void SpriteBatchRenderer::CreateBuffers()
{
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	glGenBuffers(1, &m_EBO);

	glBindVertexArray(m_VAO);

	// Vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * sizeof(SpriteVertex), nullptr, GL_DYNAMIC_DRAW);

	// Index buffer (same indices for all quads)
	std::vector<GLuint> indices;
	indices.reserve(MAX_INDICES);

	for(u32 i = 0; i < MAX_SPRITES; ++i)
	{
		GLuint base = i * 4;

		// Triangle 1
		indices.push_back(base + 0);
		indices.push_back(base + 1);
		indices.push_back(base + 2);

		// Triangle 2
		indices.push_back(base + 2);
		indices.push_back(base + 3);
		indices.push_back(base + 0);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

	// Vertex attributes
	// Position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), (void*)offsetof(SpriteVertex, position));

	// Texture coordinates
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), (void*)offsetof(SpriteVertex, texCoords));

	// Color
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), (void*)offsetof(SpriteVertex, color));

	// Texture ID
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), (void*)offsetof(SpriteVertex, texId));

	glBindVertexArray(0);
}

void SpriteBatchRenderer::CreateShader()
{
	std::string vertex_source = R"(
			#version 330 core
			layout (location = 0) in vec2 aPosition;
			layout (location = 1) in vec2 aTexCoords;
			layout (location = 2) in vec4 aColor;
			layout (location = 3) in float aTextureId;

			uniform mat4 uViewProjection;

			out vec2 TexCoords;
			out vec4 Color;
			out float TextureId;

			void main() {
				TexCoords = aTexCoords;
				Color = aColor;
				TextureId = aTextureId;
    
				gl_Position = uViewProjection * vec4(aPosition, 0.0, 1.0);
			}
			)";

	std::string fragment_source = R"(
			#version 330 core
			out vec4 FragColor;

			in vec2 TexCoords;
			in vec4 Color;
			in float TextureId;

			uniform sampler2D uTextures[32];

			void main() {
				vec4 texColor = vec4(1.0);
    
				// Sample from the correct texture slot
				int texIndex = int(TextureId);
				
				// Use a switch statement or if-else chain for better performance
				if (texIndex == 0) texColor = texture(uTextures[0], TexCoords);
				else if (texIndex == 1) texColor = texture(uTextures[1], TexCoords);
				else if (texIndex == 2) texColor = texture(uTextures[2], TexCoords);
				else if (texIndex == 3) texColor = texture(uTextures[3], TexCoords);
				else if (texIndex == 4) texColor = texture(uTextures[4], TexCoords);
				else if (texIndex == 5) texColor = texture(uTextures[5], TexCoords);
				else if (texIndex == 6) texColor = texture(uTextures[6], TexCoords);
				else if (texIndex == 7) texColor = texture(uTextures[7], TexCoords);
				else if (texIndex == 8) texColor = texture(uTextures[8], TexCoords);
				else if (texIndex == 9) texColor = texture(uTextures[9], TexCoords);
				else if (texIndex == 10) texColor = texture(uTextures[10], TexCoords);
				else if (texIndex == 11) texColor = texture(uTextures[11], TexCoords);
				else if (texIndex == 12) texColor = texture(uTextures[12], TexCoords);
				else if (texIndex == 13) texColor = texture(uTextures[13], TexCoords);
				else if (texIndex == 14) texColor = texture(uTextures[14], TexCoords);
				else if (texIndex == 15) texColor = texture(uTextures[15], TexCoords);
				// Add more slots as needed, or use texture array indexing if supported
    
				FragColor = texColor * Color;
			}
			)";

	m_shader.Init(vertex_source, fragment_source);
}


f32 SpriteBatchRenderer::GetTextureIndex(GLuint texId)
{
	if(texId == 0.0f)
	{
		return 0.0f;
	}

	// Check if texture is already in batch
	for(u32 i = 0; i < m_textureSlots.size(); ++i)
	{
		if(m_textureSlots[i] == texId)
		{
			return static_cast<f32>(i);
		}
	}

	// Add new texture if we have space
	if(m_currentTextureSlot < MAX_TEXTURES)
	{
		m_textureSlots.push_back(texId);
		return static_cast<f32>(m_currentTextureSlot++);
	}

	// No space - need to flush
	return -1.0f;
}

void SpriteBatchRenderer::Flush()
{
	if(m_vertices.empty())
	{
		return;
	}

	// Upload vertex data
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, m_vertices.size() * sizeof(SpriteVertex), m_vertices.data());

	// Bind textures
	for(u32 i = 0; i < m_textureSlots.size(); ++i)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureSlots[i]);
	}

	// Draw
	glBindVertexArray(m_VAO);
	u32 sprite_count = static_cast<u32>(m_vertices.size()) / 4;
	glDrawElements(GL_TRIANGLES, sprite_count * 6, GL_UNSIGNED_INT, 0);

	m_renderStats.drawCalls++;
	m_renderStats.verticesDrawn += static_cast<u32>(m_vertices.size());

	// Reset for next batch
	m_vertices.clear();
	m_textureSlots.clear();

	m_textureSlots.push_back(m_whiteTexture.GetTextureID());
	m_currentTextureSlot = 1;
}
