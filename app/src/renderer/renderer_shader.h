#pragma once

#include "core_log.h"

#include <GL/glew.h>

#include <string>
#include <unordered_map>

// PERF: Have Shaders loaded on a Pool or Something so that we can load them on a Job
class Shader
{
public:
	Shader() = default;

	void Init( const std::string& vertexSrc, const std::string& fragmentSrc )
	{
		GLuint vertexShader = CompileShader( GL_VERTEX_SHADER, vertexSrc );
		GLuint fragmentShader = CompileShader( GL_FRAGMENT_SHADER, fragmentSrc );

		programId = glCreateProgram();
		glAttachShader( programId, vertexShader );
		glAttachShader( programId, fragmentShader );
		glLinkProgram( programId );

		GLint success;
		glGetProgramiv( programId, GL_LINK_STATUS, &success );
		if(!success)
		{
			char buff[512];
			glGetProgramInfoLog( programId, 512, nullptr, buff );
			LOG_ERROR( "Shader linking failed: %s", buff );
		}

		glDeleteShader( vertexShader );
		glDeleteShader( fragmentShader );
	}

	void Clear()
	{
		if(programId)
		{
			glDeleteProgram( programId );
		}
	}

	Shader( const Shader& ) = delete;
	Shader& operator=( const Shader& ) = delete;

	void Use() const { glUseProgram( programId ); }

	GLint GetUniformLocation( const std::string& name ) const
	{
		auto it = uniform_locations.find( name );
		if(it != uniform_locations.end())
		{
			return it->second;
		}
		GLint location = glGetUniformLocation( programId, name.c_str() );
		uniform_locations[name] = location;
		return location;
	}

	void SetMat4( const std::string& name, const glm::mat4& matrix ) const
	{
		glUniformMatrix4fv( GetUniformLocation( name ), 1, GL_FALSE, glm::value_ptr( matrix ) );
	}

	void SetVec3( const std::string& name, const glm::vec3& vector ) const
	{
		glUniform3fv( GetUniformLocation( name ), 1, glm::value_ptr( vector ) );
	}

	void SetFloat( const std::string& name, float value ) const
	{
		glUniform1f( GetUniformLocation( name ), value );
	}

	void SetInt( const std::string& name, int value ) const
	{
		glUniform1i( GetUniformLocation( name ), value );
	}

	void SetIntArray( const std::string& name, int* values, int count ) const
	{
		glUniform1iv( GetUniformLocation( name ), count, values );
	}

private:
	GLuint CompileShader( GLenum type, const std::string& source )
	{
		GLuint shader = glCreateShader( type );
		const char* src = source.c_str();
		glShaderSource( shader, 1, &src, nullptr );
		glCompileShader( shader );

		GLint success;
		glGetShaderiv( shader, GL_COMPILE_STATUS, &success );
		if(!success)
		{
			char buff[512];
			glGetShaderInfoLog( shader, 512, nullptr, buff );
			LOG_ERROR( "Shader compilation failed: %s", buff );
		}
		return shader;
	}

	mutable std::unordered_map<std::string, GLint> uniform_locations;
	GLuint programId = 0;
};
