#include "GLShader.h"
#include "Ignis/Asset/VFS.h"
#include <glad/glad.h>

namespace ignis
{

	std::string InjectStageDefine(const std::string& source, const std::string& define_line)
	{
		if (define_line.empty())
		{
			return source;
		}

		const std::string version_token = "#version";
		const std::size_t version_pos = source.find(version_token);

		if (version_pos != std::string::npos)
		{
			const std::size_t line_end = source.find('\n', version_pos);
			if (line_end != std::string::npos)
			{
				std::string result;
				result.reserve(source.size() + define_line.size() + 1);
				result.append(source.c_str(), line_end + 1);
				result.append(define_line);
				if (!define_line.empty() && define_line.back() != '\n')
				{
					result.push_back('\n');
				}
				result.append(source.c_str() + line_end + 1);
				return result;
			}
		}

		std::string result = define_line;
		if (!define_line.empty() && define_line.back() != '\n')
		{
			result.push_back('\n');
		}
		result.append(source);
		return result;
	}

	GLuint CompileShaderStage(GLenum shader_type,
		const std::string& source,
		const std::string& define_line,
		const char* stage_label)
	{
		const GLuint shader = glCreateShader(shader_type);
		if (shader == 0)
		{
			Log::Error("ERROR::SHADER::{0}::CREATE_FAILED", stage_label);
			return 0;
		}

		const std::string final_source = InjectStageDefine(source, define_line);
		const GLchar* src = final_source.c_str();

		glShaderSource(shader, 1, &src, nullptr);
		glCompileShader(shader);

		GLint compile_status = GL_FALSE;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);

		if (compile_status == GL_FALSE)
		{
			GLint log_length = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);

			std::vector<GLchar> info_log(static_cast<std::size_t>(log_length));
			if (log_length > 0)
			{
				glGetShaderInfoLog(shader, log_length, &log_length, info_log.data());
			}

			glDeleteShader(shader);

			Log::Error("ERROR::SHADER::{0}::COMPILATION_FAILED", stage_label);
			if (!info_log.empty())
			{
				Log::Error("{0}", info_log.data());
			}

			return 0;
		}

		return shader;
	}

	GLuint LinkProgram(GLuint vertex_shader, GLuint fragment_shader)
	{
		const GLuint program = glCreateProgram();
		if (program == 0)
		{
			Log::Error("ERROR::SHADER::PROGRAM::CREATE_FAILED");
			glDeleteShader(vertex_shader);
			glDeleteShader(fragment_shader);
			return 0;
		}

		glAttachShader(program, vertex_shader);
		glAttachShader(program, fragment_shader);
		glLinkProgram(program);

		GLint linkStatus = GL_FALSE;
		glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

		if (linkStatus == GL_FALSE)
		{
			GLint logLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

			std::vector<GLchar> infoLog(static_cast<std::size_t>(logLength));
			if (logLength > 0)
			{
				glGetProgramInfoLog(program, logLength, &logLength, infoLog.data());
			}

			glDetachShader(program, vertex_shader);
			glDetachShader(program, fragment_shader);
			glDeleteProgram(program);
			glDeleteShader(vertex_shader);
			glDeleteShader(fragment_shader);

			Log::Error("ERROR::SHADER::PROGRAM::LINK_FAILED");
			if (!infoLog.empty())
			{
				Log::Error("{0}", infoLog.data());
			}

			return 0;
		}

		glDetachShader(program, vertex_shader);
		glDetachShader(program, fragment_shader);
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);

		return program;
	}

	std::string ReadFileToString(const std::string& filepath)
	{
		auto file = VFS::Open(filepath);
		if (!file.IsOpen())
		{
			Log::Error("ERROR::SHADER::IO::FAILED_TO_OPEN_FILE {0}: {1}", filepath, file.GetError());
			return {};
		}

		return file.ReadText();
	}

	GLShader::GLShader(const std::string& name, const std::string& vertex_source, const std::string& fragment_source)
		: m_id(0), m_name(name)
	{
		const GLuint vertex_shader = CompileShaderStage(GL_VERTEX_SHADER, vertex_source, "", "VERTEX");
		if (vertex_shader == 0)
		{
			return;
		}

		const GLuint fragment_shader = CompileShaderStage(GL_FRAGMENT_SHADER, fragment_source, "", "FRAGMENT");
		if (fragment_shader == 0)
		{
			glDeleteShader(vertex_shader);
			return;
		}

		m_id = LinkProgram(vertex_shader, fragment_shader);
	}

	GLShader::GLShader(const std::string& filepath)
		: m_id(0), m_name(filepath)
	{
		const std::string combined_source = ReadFileToString(filepath);
		if (combined_source.empty())
		{
			return;
		}

		const std::string vertex_define = "#define VERTEX_STAGE 1\n";
		const std::string fragment_define = "#define FRAGMENT_STAGE 1\n";

		const GLuint vertex_shader = CompileShaderStage(GL_VERTEX_SHADER, combined_source, vertex_define, "VERTEX");
		if (vertex_shader == 0)
		{
			return;
		}

		const GLuint fragment_shader = CompileShaderStage(GL_FRAGMENT_SHADER, combined_source, fragment_define, "FRAGMENT");
		if (fragment_shader == 0)
		{
			glDeleteShader(vertex_shader);
			return;
		}

		m_id = LinkProgram(vertex_shader, fragment_shader);
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

	const std::string& GLShader::GetName() const
	{
		return m_name;
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