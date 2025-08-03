#pragma once

#include "core_types.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>

#include <unordered_map>
#include <memory>
#include <vector>
#include <string>
#include <iostream>

typedef glm::vec2 Vec2;
typedef glm::vec3 Vec3;
typedef glm::vec4 Vec4;
typedef glm::mat4 Mat4;

struct SpriteVertex {
	Vec2 position;
	Vec2 texCoords;
	Vec4 color;
	f32 texId;  // Changed from u32 to f32
};

struct Sprite {
	Vec2 position = { 0.0f, 0.0f };
	Vec2 size = { 1.0f, 1.0f };
	f32 rotation = 0.0f;
	Vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLuint texId = 0;

	Vec2 uvMin = { 0.0f, 0.0f };
	Vec2 uvMax = { 1.0f, 1.0f };

	// Add constructors for convenience
	Sprite() = default;
	Sprite(Vec2 pos, Vec2 sz, GLuint tex = 0)
		: position(pos), size(sz), texId(tex) {
	}
};

struct Camera2D
{
	Vec2 position = { 0.0f, 0.0f };
	f32 zoom = 1.0f;
	f32 rotation = 0.0f;
	Vec2 cameraVelocity = { 0.0f, 0.0f };
	const float cameraDamping = 10.0f; // Higher = more responsive

	Mat4 GetViewMatrix() const {
		Mat4 transform = Mat4(1.0f);
		transform = glm::translate(transform, Vec3(-position, 0.0f)); // Negative for proper camera movement
		transform = glm::rotate(transform, glm::radians(-rotation), Vec3(0, 0, 1)); // Negative for proper rotation
		transform = glm::scale(transform, Vec3(zoom, zoom, 1.0f));
		return glm::inverse(transform); // Return inverse for proper view matrix
	}

	Mat4 GetProjectionMatrix(f32 width, f32 height) const {
		const f32 halfHeight = height * 0.5f;
		const f32 halfWidth = width * 0.5f;
		constexpr f32 zNear = -1.0f;
		constexpr f32 zFar = 1.0f;
		return glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, zNear, zFar);
	}
};

class Shader {
private:
	GLuint program_id = 0;
	mutable std::unordered_map<std::string, GLint> uniform_locations; // Made mutable

public:
	Shader(const std::string& vertex_source, const std::string& fragment_source) {
		GLuint vertex_shader = CompileShader(GL_VERTEX_SHADER, vertex_source);
		GLuint fragment_shader = CompileShader(GL_FRAGMENT_SHADER, fragment_source);

		program_id = glCreateProgram();
		glAttachShader(program_id, vertex_shader);
		glAttachShader(program_id, fragment_shader);
		glLinkProgram(program_id);

		// Check linking
		GLint success;
		glGetProgramiv(program_id, GL_LINK_STATUS, &success);
		if (!success) {
			char info_log[512];
			glGetProgramInfoLog(program_id, 512, nullptr, info_log);
			std::cerr << "Shader linking failed: " << info_log << std::endl;
		}

		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);
	}

	~Shader() {
		if (program_id)
			glDeleteProgram(program_id);
	}

	// Disable copy constructor and assignment
	Shader(const Shader&) = delete;
	Shader& operator=(const Shader&) = delete;

	void Use() const { glUseProgram(program_id); }

	GLint GetUniformLocation(const std::string& name) const { // Made const
		auto it = uniform_locations.find(name);
		if (it != uniform_locations.end()) {
			return it->second;
		}
		GLint location = glGetUniformLocation(program_id, name.c_str());
		uniform_locations[name] = location;
		return location;
	}

	void SetMat4(const std::string& name, const glm::mat4& matrix) const {
		glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void SetVec3(const std::string& name, const glm::vec3& vector) const {
		glUniform3fv(GetUniformLocation(name), 1, glm::value_ptr(vector));
	}

	void SetFloat(const std::string& name, float value) const {
		glUniform1f(GetUniformLocation(name), value);
	}

	void SetInt(const std::string& name, int value) const {
		glUniform1i(GetUniformLocation(name), value);
	}

	// Add SetIntArray method
	void SetIntArray(const std::string& name, int* values, int count) const {
		glUniform1iv(GetUniformLocation(name), count, values);
	}

private:
	GLuint CompileShader(GLenum type, const std::string& source) {
		GLuint shader = glCreateShader(type);
		const char* src = source.c_str();
		glShaderSource(shader, 1, &src, nullptr);
		glCompileShader(shader);

		GLint success;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success) {
			char info_log[512];
			glGetShaderInfoLog(shader, 512, nullptr, info_log);
			std::cerr << "Shader compilation failed: " << info_log << std::endl;
		}
		return shader;
	}
};

class SpriteBatchRenderer
{
public:
	struct Stats {
		u32 draw_calls = 0;
		u32 sprites_drawn = 0;
		u32 vertices_drawn = 0;

		void Reset() {
			draw_calls = 0;
			sprites_drawn = 0;
			vertices_drawn = 0;
		}
	};

	SpriteBatchRenderer() {
		CreateBuffers();
		CreateShader();

		vertices.reserve(MAX_VERTICES);
		texture_slots.reserve(MAX_TEXTURES);

		CreateWhiteTexture();
	}

	~SpriteBatchRenderer() {
		if (VAO) glDeleteVertexArrays(1, &VAO);
		if (VBO) glDeleteBuffers(1, &VBO); // Fixed: should be glDeleteBuffers, not glDeleteVertexArrays
		if (EBO) glDeleteBuffers(1, &EBO); // Fixed: should be glDeleteBuffers, not glDeleteVertexArrays
		if (white_texture) glDeleteTextures(1, &white_texture);
	}

	void Begin(const Camera2D& camera, f32 viewport_width, f32 viewport_height) {
		stats.Reset();
		vertices.clear();
		texture_slots.clear();
		current_texture_slot = 0;

		// Always have white texture in slot 0
		texture_slots.push_back(white_texture);
		current_texture_slot = 1;

		// Set up camera matrices
		shader->Use();
		Mat4 view = camera.GetViewMatrix();
		Mat4 projection = camera.GetProjectionMatrix(viewport_width, viewport_height);
		Mat4 view_projection = projection * view;

		shader->SetMat4("uViewProjection", view_projection);

		// Set up texture samplers
		int samplers[MAX_TEXTURES]; // Changed from i32 to int
		for (int i = 0; i < MAX_TEXTURES; ++i) {
			samplers[i] = i;
		}
		shader->SetIntArray("uTextures", samplers, MAX_TEXTURES);
	}

	void DrawSprite(const Sprite& sprite) {
		f32 texture_index = GetTextureIndex(sprite.texId);

		// Check if we need to flush (too many sprites or textures)
		if (vertices.size() + 4 > MAX_VERTICES ||
			(texture_index == -1.0f && current_texture_slot >= MAX_TEXTURES)) {
			Flush();

			// Restart batch - but don't call Begin() again, just reset internal state
			vertices.clear();
			texture_slots.clear();
			texture_slots.push_back(white_texture);
			current_texture_slot = 1;

			texture_index = GetTextureIndex(sprite.texId);
		}

		// Create quad vertices
		Vec2 half_size = sprite.size * 0.5f;

		// Calculate corner positions
		Vec2 corners[4] = {
			{-half_size.x, -half_size.y}, // Bottom-left
			{ half_size.x, -half_size.y}, // Bottom-right
			{ half_size.x,  half_size.y}, // Top-right
			{-half_size.x,  half_size.y}  // Top-left
		};

		// Apply rotation if needed
		if (sprite.rotation != 0.0f) {
			f32 cos_r = cos(glm::radians(sprite.rotation));
			f32 sin_r = sin(glm::radians(sprite.rotation));

			for (int i = 0; i < 4; ++i) { // Changed from i32 to int
				f32 x = corners[i].x;
				f32 y = corners[i].y;
				corners[i].x = x * cos_r - y * sin_r;
				corners[i].y = x * sin_r + y * cos_r;
			}
		}

		// Translate to world position
		for (int i = 0; i < 4; ++i) { // Changed from i32 to int
			corners[i] += sprite.position;
		}

		// UV coordinates
		Vec2 uvs[4] = {
			{sprite.uvMin.x, sprite.uvMin.y}, // Bottom-left
			{sprite.uvMax.x, sprite.uvMin.y}, // Bottom-right
			{sprite.uvMax.x, sprite.uvMax.y}, // Top-right
			{sprite.uvMin.x, sprite.uvMax.y}  // Top-left
		};

		// Add vertices to batch
		for (int i = 0; i < 4; ++i) { // Changed from i32 to int
			vertices.emplace_back(SpriteVertex{ corners[i], uvs[i], sprite.color, texture_index });
		}

		stats.sprites_drawn++;
	}

	void DrawSprite(Vec2 position, Vec2 size, GLuint texture = 0, Vec4 color = Vec4(1, 1, 1, 1)) {
		Sprite sprite(position, size, texture);
		sprite.color = color;
		DrawSprite(sprite);
	}

	void DrawRotatedSprite(Vec2 position, Vec2 size, f32 rotation, GLuint texture = 0, Vec4 color = Vec4(1, 1, 1, 1)) {
		Sprite sprite(position, size, texture);
		sprite.rotation = rotation;
		sprite.color = color;
		DrawSprite(sprite);
	}

	void End() {
		Flush();
	}

	const Stats& GetStats() const { return stats; }

private:
	static constexpr u32 MAX_SPRITES = 10000;
	static constexpr u32 MAX_VERTICES = MAX_SPRITES * 4;
	static constexpr u32 MAX_INDICES = MAX_SPRITES * 6;
	static constexpr u32 MAX_TEXTURES = 32; // Max texture slots

	// OpenGL objects
	GLuint VAO = 0, VBO = 0, EBO = 0;
	GLuint white_texture = 0;
	std::unique_ptr<Shader> shader;

	// Batch data
	std::vector<SpriteVertex> vertices;
	std::vector<GLuint> texture_slots;
	u32 current_texture_slot = 0;

	// Statistics
	Stats stats;

	void CreateBuffers() {
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

		for (u32 i = 0; i < MAX_SPRITES; ++i) {
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

	void CreateShader() {
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

		shader = std::make_unique<Shader>(vertex_source, fragment_source);
	}

	void CreateWhiteTexture() {
		glGenTextures(1, &white_texture);
		glBindTexture(GL_TEXTURE_2D, white_texture);

		unsigned char white_pixel[4] = { 255, 255, 255, 255 };
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white_pixel);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glBindTexture(GL_TEXTURE_2D, 0); // Unbind
	}

	f32 GetTextureIndex(GLuint texId) {
		// Use white texture for 0/invalid textures
		if (texId == 0) {
			return 0.0f;
		}

		// Check if texture is already in batch
		for (u32 i = 0; i < texture_slots.size(); ++i) {
			if (texture_slots[i] == texId) {
				return static_cast<f32>(i);
			}
		}

		// Add new texture if we have space
		if (current_texture_slot < MAX_TEXTURES) {
			texture_slots.push_back(texId);
			return static_cast<f32>(current_texture_slot++);
		}

		// No space - need to flush
		return -1.0f;
	}

	void Flush() {
		if (vertices.empty())
			return;

		// Upload vertex data
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(SpriteVertex), vertices.data());

		// Bind textures
		for (u32 i = 0; i < texture_slots.size(); ++i) {
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, texture_slots[i]);
		}

		// Draw
		glBindVertexArray(VAO);
		u32 sprite_count = static_cast<u32>(vertices.size()) / 4;
		glDrawElements(GL_TRIANGLES, sprite_count * 6, GL_UNSIGNED_INT, 0);

		stats.draw_calls++;
		stats.vertices_drawn += static_cast<u32>(vertices.size());

		// Reset for next batch
		vertices.clear();
		texture_slots.clear();
		texture_slots.push_back(white_texture); // Always keep white texture
		current_texture_slot = 1;
	}
};
