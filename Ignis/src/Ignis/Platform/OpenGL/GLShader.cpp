#include "GLShader.h"
#include <glad/glad.h>

namespace ignis
{

	GLShader::GLShader(const std::string& vertex_source, const std::string& fragment_source)
	{

		// Create an empty vertex shader handle
		GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);

		// Send the vertex shader source code to GL
		// Note that std::string's .c_str is NULL character terminated.
		const GLchar* source = (const GLchar*)vertex_source.c_str();
		glShaderSource(vertex_shader, 1, &source, 0);

		// Compile the vertex shader
		glCompileShader(vertex_shader);

		GLint is_compiled = 0;
		glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &is_compiled);
		if (is_compiled == GL_FALSE)
		{
			GLint max_length = 0;
			glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &max_length);

			// The maxLength includes the NULL character
			std::vector<GLchar> info_log(max_length);
			glGetShaderInfoLog(vertex_shader, max_length, &max_length, &info_log[0]);

			// We don't need the shader anymore.
			glDeleteShader(vertex_shader);

			Log::Error("ERROR::SHADER::VERTEX::COMPILATION_FAILED");
			Log::Error("{0}", info_log.data());

			// In this simple program, we'll just leave
			return;
		}

		// Create an empty fragment shader handle
		GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

		// Send the fragment shader source code to GL
		// Note that std::string's .c_str is NULL character terminated.
		source = (const GLchar*)fragment_source.c_str();
		glShaderSource(fragment_shader, 1, &source, 0);

		// Compile the fragment shader
		glCompileShader(fragment_shader);

		glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &is_compiled);
		if (is_compiled == GL_FALSE)
		{
			GLint max_length = 0;
			glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &max_length);

			// The maxLength includes the NULL character
			std::vector<GLchar> info_log(max_length);
			glGetShaderInfoLog(fragment_shader, max_length, &max_length, &info_log[0]);

			// We don't need the shader anymore.
			glDeleteShader(fragment_shader);
			// Either of them. Don't leak shaders.
			glDeleteShader(vertex_shader);

			Log::Error("ERROR::SHADER::VERTEX::COMPILATION_FAILED");
			Log::Error("{0}", info_log.data());

			// In this simple program, we'll just leave
			return;
		}

		// Vertex and fragment shaders are successfully compiled.
		// Now time to link them together into a program.
		// Get a program object.
		GLuint program = glCreateProgram();

		// Attach our shaders to our program
		glAttachShader(program, vertex_shader);
		glAttachShader(program, fragment_shader);

		// Link our program
		glLinkProgram(program);

		// Note the different functions here: glGetProgram* instead of glGetShader*.
		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint max_length = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &max_length);

			// The maxLength includes the NULL character
			std::vector<GLchar> infoLog(max_length);
			glGetProgramInfoLog(program, max_length, &max_length, &infoLog[0]);

			// We don't need the program anymore.
			glDeleteProgram(program);
			// Don't leak shaders either.
			glDeleteShader(vertex_shader);
			glDeleteShader(fragment_shader);

			// Use the infoLog as you see fit.

			// In this simple program, we'll just leave
			return;
		}

		// Always detach shaders after a successful link.
		glDetachShader(program, vertex_shader);
		glDetachShader(program, fragment_shader);
	}

	GLShader::GLShader(const std::string& filepath)
	{
		// TODO: Implement shader loading from file
	}

	GLShader::~GLShader()
	{
		glDeleteProgram(m_id);
	}

	void GLShader::Bind()
	{
		glUseProgram(m_id);
	}

	void GLShader::UnBind()
	{
		glUseProgram(0);
	}

	void GLShader::Set(const std::string& name, float value)
	{
		glUniform1f(glGetUniformLocation(m_id, name.c_str()), value);
	}

	void GLShader::Set(const std::string& name, const glm::vec2& vector)
	{
		glUniform2fv(glGetUniformLocation(m_id, name.c_str()), 1, &vector[0]);
	}

	void GLShader::Set(const std::string& name, const glm::vec3& vector)
	{
		glUniform3fv(glGetUniformLocation(m_id, name.c_str()), 1, &vector[0]);
	}

	void GLShader::Set(const std::string& name, const glm::vec4& vector)
	{
		glUniform4fv(glGetUniformLocation(m_id, name.c_str()), 1, &vector[0]);
	}

	void GLShader::Set(const std::string& name, int value)
	{
		glUniform1i(glGetUniformLocation(m_id, name.c_str()), value);
	}

	void GLShader::Set(const std::string& name, const glm::ivec2& vector)
	{
		glUniform2iv(glGetUniformLocation(m_id, name.c_str()), 1, &vector[0]);
	}

	void GLShader::Set(const std::string& name, const glm::ivec3& vector)
	{
		glUniform3iv(glGetUniformLocation(m_id, name.c_str()), 1, &vector[0]);
	}

	void GLShader::Set(const std::string& name, const glm::ivec4& vector)
	{
		glUniform4iv(glGetUniformLocation(m_id, name.c_str()), 1, &vector[0]);
	}

	void GLShader::Set(const std::string& name, unsigned int value)
	{
		glUniform1ui(glGetUniformLocation(m_id, name.c_str()), value);
	}

	void GLShader::Set(const std::string& name, const glm::uvec2& vector)
	{
		glUniform2uiv(glGetUniformLocation(m_id, name.c_str()), 1, &vector[0]);
	}

	void GLShader::Set(const std::string& name, const glm::uvec3& vector)
	{
		glUniform3uiv(glGetUniformLocation(m_id, name.c_str()), 1, &vector[0]);
	}

	void GLShader::Set(const std::string& name, const glm::uvec4& vector)
	{
		glUniform4uiv(glGetUniformLocation(m_id, name.c_str()), 1, &vector[0]);
	}

	void GLShader::Set(const std::string& name, const glm::mat3& matrix)
	{
		glUniformMatrix3fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, &matrix[0][0]);
	}

	void GLShader::Set(const std::string& name, const glm::mat4& matrix)
	{
		glUniformMatrix4fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, &matrix[0][0]);
	}
}