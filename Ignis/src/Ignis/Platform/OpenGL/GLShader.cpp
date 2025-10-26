#include "GLShader.h"
#include "Ignis/Asset/VFS.h"
#include <glad/glad.h>

namespace ignis
{

	std::string InjectStageDefine(const std::string& source, const std::string& defineLine)
	{
		if (defineLine.empty())
		{
			return source;
		}

		const std::string versionToken = "#version";
		const std::size_t versionPos = source.find(versionToken);

		if (versionPos != std::string::npos)
		{
			const std::size_t lineEnd = source.find('\n', versionPos);
			if (lineEnd != std::string::npos)
			{
				std::string result;
				result.reserve(source.size() + defineLine.size() + 1);
				result.append(source.c_str(), lineEnd + 1);
				result.append(defineLine);
				if (!defineLine.empty() && defineLine.back() != '\n')
				{
					result.push_back('\n');
				}
				result.append(source.c_str() + lineEnd + 1);
				return result;
			}
		}

		std::string result = defineLine;
		if (!defineLine.empty() && defineLine.back() != '\n')
		{
			result.push_back('\n');
		}
		result.append(source);
		return result;
	}

	GLuint CompileShaderStage(GLenum shaderType,
		const std::string& source,
		const std::string& defineLine,
		const char* stageLabel)
	{
		const GLuint shader = glCreateShader(shaderType);
		if (shader == 0)
		{
			Log::Error("ERROR::SHADER::{0}::CREATE_FAILED", stageLabel);
			return 0;
		}

		const std::string finalSource = InjectStageDefine(source, defineLine);
		const GLchar* src = finalSource.c_str();

		glShaderSource(shader, 1, &src, nullptr);
		glCompileShader(shader);

		GLint compileStatus = GL_FALSE;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);

		if (compileStatus == GL_FALSE)
		{
			GLint logLength = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

			std::vector<GLchar> infoLog(static_cast<std::size_t>(logLength));
			if (logLength > 0)
			{
				glGetShaderInfoLog(shader, logLength, &logLength, infoLog.data());
			}

			glDeleteShader(shader);

			Log::Error("ERROR::SHADER::{0}::COMPILATION_FAILED", stageLabel);
			if (!infoLog.empty())
			{
				Log::Error("{0}", infoLog.data());
			}

			return 0;
		}

		return shader;
	}

	GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader)
	{
		const GLuint program = glCreateProgram();
		if (program == 0)
		{
			Log::Error("ERROR::SHADER::PROGRAM::CREATE_FAILED");
			glDeleteShader(vertexShader);
			glDeleteShader(fragmentShader);
			return 0;
		}

		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);
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

			glDetachShader(program, vertexShader);
			glDetachShader(program, fragmentShader);
			glDeleteProgram(program);
			glDeleteShader(vertexShader);
			glDeleteShader(fragmentShader);

			Log::Error("ERROR::SHADER::PROGRAM::LINK_FAILED");
			if (!infoLog.empty())
			{
				Log::Error("{0}", infoLog.data());
			}

			return 0;
		}

		glDetachShader(program, vertexShader);
		glDetachShader(program, fragmentShader);
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

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

	GLShader::GLShader(const std::string& vertex_source, const std::string& fragment_source)
		: m_id(0)
	{
		const GLuint vertexShader = CompileShaderStage(GL_VERTEX_SHADER, vertex_source, "", "VERTEX");
		if (vertexShader == 0)
		{
			return;
		}

		const GLuint fragmentShader = CompileShaderStage(GL_FRAGMENT_SHADER, fragment_source, "", "FRAGMENT");
		if (fragmentShader == 0)
		{
			glDeleteShader(vertexShader);
			return;
		}

		m_id = LinkProgram(vertexShader, fragmentShader);
	}

	GLShader::GLShader(const std::string& filepath)
		: m_id(0)
	{
		const std::string combinedSource = ReadFileToString(filepath);
		if (combinedSource.empty())
		{
			return;
		}

		const std::string vertexDefine = "#define VERTEX_STAGE 1\n";
		const std::string fragmentDefine = "#define FRAGMENT_STAGE 1\n";

		const GLuint vertexShader = CompileShaderStage(GL_VERTEX_SHADER, combinedSource, vertexDefine, "VERTEX");
		if (vertexShader == 0)
		{
			return;
		}

		const GLuint fragmentShader = CompileShaderStage(GL_FRAGMENT_SHADER, combinedSource, fragmentDefine, "FRAGMENT");
		if (fragmentShader == 0)
		{
			glDeleteShader(vertexShader);
			return;
		}

		m_id = LinkProgram(vertexShader, fragmentShader);
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