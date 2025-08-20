#pragma once

#include "core/core_minimal.h"

#include <GL/glew.h>

#include <string>
#include <unordered_map>

#include "types.h"

// PERF: Have Shaders loaded on a Pool or Something so that we can load them on a Job
class Shader
{
public:
	Shader() = default;

	void Init( const std::string& vertexSrc, const std::string& fragmentSrc );
	void Clear();

	Shader( const Shader& ) = delete;
	Shader& operator=( const Shader& ) = delete;

	void Use() const;

	GLint GetUniformLocation( const std::string& name ) const;

	void SetMat4( const std::string& name, const Mat4& matrix ) const;
	void SetVec3( const std::string& name, const Vec3& vector ) const;
	void SetFloat( const std::string& name, float value ) const;
	void SetInt( const std::string& name, i32 value ) const;
	void SetIntArray( const std::string& name, i32* values, i32 count ) const;

private:
	GLuint CompileShader( GLenum type, const std::string& source );

	mutable std::unordered_map<std::string, GLint> uniform_locations;
	GLuint programId = 0;
};
