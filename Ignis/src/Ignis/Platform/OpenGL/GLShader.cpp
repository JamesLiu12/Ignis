#include "GLShader.h"
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

	static ShaderUniformType GLTypeToUniformType(GLenum type)
	{
		switch (type)
		{
		case GL_BOOL:              return ShaderUniformType::Bool;
		case GL_FLOAT:             return ShaderUniformType::Float;
		case GL_FLOAT_VEC2:        return ShaderUniformType::Vec2;
		case GL_FLOAT_VEC3:        return ShaderUniformType::Vec3;
		case GL_FLOAT_VEC4:        return ShaderUniformType::Vec4;
		case GL_INT:               return ShaderUniformType::Int;
		case GL_INT_VEC2:          return ShaderUniformType::IVec2;
		case GL_INT_VEC3:          return ShaderUniformType::IVec3;
		case GL_INT_VEC4:          return ShaderUniformType::IVec4;
		case GL_UNSIGNED_INT:      return ShaderUniformType::UInt;
		case GL_UNSIGNED_INT_VEC2: return ShaderUniformType::UVec2;
		case GL_UNSIGNED_INT_VEC3: return ShaderUniformType::UVec3;
		case GL_UNSIGNED_INT_VEC4: return ShaderUniformType::UVec4;
		case GL_FLOAT_MAT3:        return ShaderUniformType::Mat3;
		case GL_FLOAT_MAT4:        return ShaderUniformType::Mat4;
		default:                   return ShaderUniformType::None;
		}
	}

	static bool GLTypeIsSampler(GLenum type)
	{
		switch (type)
		{
		case GL_SAMPLER_2D:
		case GL_SAMPLER_3D:
		case GL_SAMPLER_CUBE:
		case GL_SAMPLER_2D_ARRAY:
		case GL_SAMPLER_2D_SHADOW:
			return true;
		default:
			return false;
		}
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

		if (m_id != 0)
			Reflect();
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

		if (m_id != 0)
			Reflect();
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

	void GLShader::Reflect()
	{
		if (m_id == 0) return;

		GLint uniformCount = 0;
		glGetProgramiv(m_id, GL_ACTIVE_UNIFORMS, &uniformCount);

		m_uniformBufferSize = 0;
		uint32_t samplerSlot = 0;

		for (GLint i = 0; i < uniformCount; ++i)
		{
			GLchar  nameBuffer[256];
			GLsizei nameLength = 0;
			GLint   arraySize = 0;
			GLenum  glType = 0;

			glGetActiveUniform(m_id, static_cast<GLuint>(i),
				sizeof(nameBuffer), &nameLength,
				&arraySize, &glType, nameBuffer);

			std::string name(nameBuffer, static_cast<size_t>(nameLength));

			if (name.size() > 3 && name.compare(name.size() - 3, 3, "[0]") == 0)
				name.resize(name.size() - 3);

			if (GLTypeIsSampler(glType))
			{
				m_samplers[name] = { name, samplerSlot++ };
			}
			else
			{
				ShaderUniformType type = GLTypeToUniformType(glType);
				if (type == ShaderUniformType::None)
					continue;

				ShaderUniform u;
				u.name = name;
				u.type = type;
				u.size = ShaderUniform::TypeSize(type);
				u.offset = m_uniformBufferSize;
				m_uniformBufferSize += u.size;

				m_uniforms[name] = u;
			}
		}
	}

	int32_t GLShader::GetUniformLocation(const std::string& name) const
	{
		auto it = m_locationCache.find(name);
		if (it != m_locationCache.end())
			return it->second;

		int32_t loc = glGetUniformLocation(m_id, name.c_str());
		m_locationCache[name] = loc;
		return loc;
	}
}