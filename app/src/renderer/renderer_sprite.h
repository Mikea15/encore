#pragma once

#include "core/core_types.h"

#include "render2d_types.h"
#include "render2d_camera.h"
#include "renderer_shader.h"

#include <GL/glew.h>

#include <string>
#include <vector>
#include <entity/entity.h>

class SpriteBatchRenderer
{
public:
	struct Stats
	{
		u32 drawCalls = 0;
		u32 spritesDrawn = 0;
		u32 verticesDrawn = 0;

		void Reset()
		{
			drawCalls = 0;
			spritesDrawn = 0;
			verticesDrawn = 0;
		}
	};

	SpriteBatchRenderer() = default;

	void Init()
	{
		CreateBuffers();
		CreateShader();

		vertices.reserve(MAX_VERTICES);
		textureSlots.reserve(MAX_TEXTURES);

		CreateWhiteTexture();
	}

	void Clear()
	{
		shader.Clear();

		if(VAO) { glDeleteVertexArrays(1, &VAO); }
		if(VBO) { glDeleteBuffers(1, &VBO); }
		if(EBO) { glDeleteBuffers(1, &EBO); }
		if(whiteTexture) { glDeleteTextures(1, &whiteTexture); }
	}

	void Begin(const Camera2D& cam, f32 viewportWidth, f32 viewportHeight)
	{
		stats.Reset();
		vertices.clear();
		textureSlots.clear();
		currentTextureSlot = 0;

		// Always have white texture in slot 0
		textureSlots.push_back(whiteTexture);
		currentTextureSlot = 1;

		// Set up camera matrices
		shader.Use();
		const Mat4 view = cam.GetViewMatrix();
		const Mat4 projection = cam.GetProjectionMatrix(viewportWidth, viewportHeight);
		const Mat4 viewProjection = projection * view;

		shader.SetMat4("uViewProjection", viewProjection);

		// Set up texture samplers
		i32 samplers[MAX_TEXTURES];
		for(i32 i = 0; i < MAX_TEXTURES; ++i)
		{
			samplers[i] = i;
		}
		shader.SetIntArray("uTextures", samplers, MAX_TEXTURES);
	}

	void DrawSprite(const Vec2& position, float rotation, const Sprite& sprite)
	{
		f32 textureIndex = GetTextureIndex(sprite.texId);

		// Check if we need to flush (too many sprites or textures)
		if(vertices.size() + 4 > MAX_VERTICES ||
			(textureIndex == -1.0f && currentTextureSlot >= MAX_TEXTURES))
		{
			Flush();

			// Restart batch - but don't call Begin() again, just reset internal state
			vertices.clear();
			textureSlots.clear();
			textureSlots.push_back(whiteTexture);
			currentTextureSlot = 1;

			textureIndex = GetTextureIndex(sprite.texId);
		}

		// Create quad vertices
		Vec2 halfSize = sprite.size * 0.5f;

		// Calculate corner positions
		Vec2 corners[4] = {
			{-halfSize.x, -halfSize.y}, // Bottom-left
			{ halfSize.x, -halfSize.y}, // Bottom-right
			{ halfSize.x,  halfSize.y}, // Top-right
			{-halfSize.x,  halfSize.y}  // Top-left
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
			{sprite.uvMin.x, sprite.uvMin.y}, // Bottom-left
			{sprite.uvMax.x, sprite.uvMin.y}, // Bottom-right
			{sprite.uvMax.x, sprite.uvMax.y}, // Top-right
			{sprite.uvMin.x, sprite.uvMax.y}  // Top-left
		};

		// Add vertices to batch
		for(i8 i = 0; i < 4; ++i)
		{
			vertices.emplace_back(SpriteVertex{ corners[i], uvs[i], sprite.color, textureIndex });
		}

		stats.spritesDrawn++;
	}

	// PERF: Move Semantics Here?
	void DrawSprite(Vec2 position, Vec2 size, GLuint texture = 0, Vec4 color = Vec4(1, 1, 1, 1))
	{
		Sprite sprite(position, size, texture);
		sprite.color = color;
		DrawSprite(position, sprite.rotation, sprite);
	}

	// PERF: Move Semantics Here?
	void DrawRotatedSprite(Vec2 position, Vec2 size, f32 rotation, GLuint texture = 0, Vec4 color = Vec4(1, 1, 1, 1))
	{
		Sprite sprite(position, size, texture);
		sprite.rotation = rotation;
		sprite.color = color;
		DrawSprite(position, sprite.rotation, sprite);
	}

	void End()
	{
		Flush();
	}

	const Stats& GetStats() const { return stats; }

private:
	void CreateBuffers()
	{
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);

		// Vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
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

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
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

	void CreateShader()
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

		shader.Init(vertex_source, fragment_source);
	}

	void CreateWhiteTexture()
	{
		glGenTextures(1, &whiteTexture);
		glBindTexture(GL_TEXTURE_2D, whiteTexture);

		u8 white_pixel[4] = { 255, 255, 255, 255 };
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white_pixel);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glBindTexture(GL_TEXTURE_2D, 0); // Unbind
	}

	f32 GetTextureIndex(GLuint texId)
	{
		// Use white texture for 0/invalid textures
		if(texId == 0)
		{
			return 0.0f;
		}

		// Check if texture is already in batch
		for(u32 i = 0; i < textureSlots.size(); ++i)
		{
			if(textureSlots[i] == texId)
			{
				return static_cast<f32>(i);
			}
		}

		// Add new texture if we have space
		if(currentTextureSlot < MAX_TEXTURES)
		{
			textureSlots.push_back(texId);
			return static_cast<f32>(currentTextureSlot++);
		}

		// No space - need to flush
		return -1.0f;
	}

	void Flush()
	{
		if(vertices.empty())
		{
			return;
		}

		// Upload vertex data
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(SpriteVertex), vertices.data());

		// Bind textures
		for(u32 i = 0; i < textureSlots.size(); ++i)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, textureSlots[i]);
		}

		// Draw
		glBindVertexArray(VAO);
		u32 sprite_count = static_cast<u32>(vertices.size()) / 4;
		glDrawElements(GL_TRIANGLES, sprite_count * 6, GL_UNSIGNED_INT, 0);

		stats.drawCalls++;
		stats.verticesDrawn += static_cast<u32>(vertices.size());

		// Reset for next batch
		vertices.clear();
		textureSlots.clear();
		textureSlots.push_back(whiteTexture); // Always keep white texture
		currentTextureSlot = 1;
	}

	static constexpr u32 MAX_SPRITES = 100000;
	static constexpr u32 MAX_VERTICES = MAX_SPRITES * 4;
	static constexpr u32 MAX_INDICES = MAX_SPRITES * 6;
	static constexpr u8 MAX_TEXTURES = 32; // Max texture slots

	// OpenGL objects
	GLuint VAO = 0, VBO = 0, EBO = 0;
	GLuint whiteTexture = 0;
	Shader shader;

	// Batch data
	std::vector<SpriteVertex> vertices;
	std::vector<GLuint> textureSlots;
	u32 currentTextureSlot = 0;

	// Statistics
	Stats stats;
};

struct Render2DInfo
{
	SpriteBatchRenderer renderer;
	Camera2D camera;
	std::vector<Entity> entities;
};
