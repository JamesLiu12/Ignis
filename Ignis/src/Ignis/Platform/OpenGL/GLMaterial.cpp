#include "GLMaterial.h"
#include <glad/glad.h>

namespace ignis
{
	GLMaterial::GLMaterial(std::shared_ptr<Shader> shader, const std::string& name)
		: m_shader(std::dynamic_pointer_cast<GLShader>(std::move(shader)))
		, m_name(name)
	{
		m_uniformBuffer.resize(m_shader->GetUniformBufferSize(), 0);
	}

	GLMaterial::GLMaterial(const std::shared_ptr<Material>& other, const std::string& name)
		: m_shader(std::dynamic_pointer_cast<GLShader>(other->GetShader()))
		, m_name(name)
	{
		m_uniformBuffer.resize(m_shader->GetUniformBufferSize(), 0);

		if (auto* src = dynamic_cast<const GLMaterial*>(other.get()))
		{
			m_uniformBuffer = src->m_uniformBuffer;
			m_textures = src->m_textures;
		}
	}

	void GLMaterial::SetBytes(const std::string& name, const void* data, uint32_t size)
	{
		auto it = m_shader->GetUniforms().find(name);
		if (it == m_shader->GetUniforms().end())
		{
			Log::Warn("GLMaterial::Set: uniform '{0}' does not exist in shader '{1}'",
				name, m_shader->GetName());
			return;
		}
		const ShaderUniform& u = it->second;
		assert(u.size == size && "The data size of the Set does not match the uniform type");
		std::memcpy(m_uniformBuffer.data() + u.offset, data, size);
	}

	void* GLMaterial::GetBytes(const std::string& name)
	{
		auto it = m_shader->GetUniforms().find(name);
		assert(it != m_shader->GetUniforms().end() && "Uniform does not exist");
		return m_uniformBuffer.data() + it->second.offset;
	}

	void GLMaterial::Set(const std::string& name, bool value)
	{
		int32_t v = value ? 1 : 0;
		SetBytes(name, &v, sizeof(v));
	}
	void GLMaterial::Set(const std::string& name, float value) { SetBytes(name, &value, sizeof(value)); }
	void GLMaterial::Set(const std::string& name, const glm::vec2& value) { SetBytes(name, &value[0], sizeof(value)); }
	void GLMaterial::Set(const std::string& name, const glm::vec3& value) { SetBytes(name, &value[0], sizeof(value)); }
	void GLMaterial::Set(const std::string& name, const glm::vec4& value) { SetBytes(name, &value[0], sizeof(value)); }
	void GLMaterial::Set(const std::string& name, int value) { SetBytes(name, &value, sizeof(value)); }
	void GLMaterial::Set(const std::string& name, const glm::ivec2& value) { SetBytes(name, &value[0], sizeof(value)); }
	void GLMaterial::Set(const std::string& name, const glm::ivec3& value) { SetBytes(name, &value[0], sizeof(value)); }
	void GLMaterial::Set(const std::string& name, const glm::ivec4& value) { SetBytes(name, &value[0], sizeof(value)); }
	void GLMaterial::Set(const std::string& name, uint32_t value) { SetBytes(name, &value, sizeof(value)); }
	void GLMaterial::Set(const std::string& name, const glm::uvec2& value) { SetBytes(name, &value[0], sizeof(value)); }
	void GLMaterial::Set(const std::string& name, const glm::uvec3& value) { SetBytes(name, &value[0], sizeof(value)); }
	void GLMaterial::Set(const std::string& name, const glm::uvec4& value) { SetBytes(name, &value[0], sizeof(value)); }
	void GLMaterial::Set(const std::string& name, const glm::mat3& value) { SetBytes(name, &value[0][0], sizeof(value)); }
	void GLMaterial::Set(const std::string& name, const glm::mat4& value) { SetBytes(name, &value[0][0], sizeof(value)); }

	void GLMaterial::Set(const std::string& name, const std::shared_ptr<Texture2D>& texture)
	{
		if (m_shader->GetSamplers().find(name) == m_shader->GetSamplers().end())
		{
			Log::Warn("GLMaterial::Set: sampler '{}' does not exist in shader '{}'",
				name, m_shader->GetName());
			return;
		}
		m_textures[name] = texture;
	}

	float& GLMaterial::GetFloat(const std::string& name) { return GetValue<float>(name); }
	int32_t& GLMaterial::GetInt(const std::string& name) { return GetValue<int32_t>(name); }
	uint32_t& GLMaterial::GetUInt(const std::string& name) { return GetValue<uint32_t>(name); }
	glm::vec2& GLMaterial::GetVec2(const std::string& name) { return GetValue<glm::vec2>(name); }
	glm::vec3& GLMaterial::GetVec3(const std::string& name) { return GetValue<glm::vec3>(name); }
	glm::vec4& GLMaterial::GetVec4(const std::string& name) { return GetValue<glm::vec4>(name); }
	glm::mat3& GLMaterial::GetMat3(const std::string& name) { return GetValue<glm::mat3>(name); }
	glm::mat4& GLMaterial::GetMat4(const std::string& name) { return GetValue<glm::mat4>(name); }

	void GLMaterial::UploadUniforms()
	{
		for (const auto& [name, u] : m_shader->GetUniforms())
		{
			int32_t loc = m_shader->GetUniformLocation(name);
			if (loc == -1) continue;

			const void* data = m_uniformBuffer.data() + u.offset;

			switch (u.type)
			{
			case ShaderUniformType::Bool:
			case ShaderUniformType::Int:   glUniform1iv(loc, 1, (const int32_t*)data);  break;
			case ShaderUniformType::IVec2: glUniform2iv(loc, 1, (const int32_t*)data);  break;
			case ShaderUniformType::IVec3: glUniform3iv(loc, 1, (const int32_t*)data);  break;
			case ShaderUniformType::IVec4: glUniform4iv(loc, 1, (const int32_t*)data);  break;
			case ShaderUniformType::UInt:  glUniform1uiv(loc, 1, (const uint32_t*)data); break;
			case ShaderUniformType::UVec2: glUniform2uiv(loc, 1, (const uint32_t*)data); break;
			case ShaderUniformType::UVec3: glUniform3uiv(loc, 1, (const uint32_t*)data); break;
			case ShaderUniformType::UVec4: glUniform4uiv(loc, 1, (const uint32_t*)data); break;
			case ShaderUniformType::Float: glUniform1fv(loc, 1, (const float*)data);    break;
			case ShaderUniformType::Vec2:  glUniform2fv(loc, 1, (const float*)data);    break;
			case ShaderUniformType::Vec3:  glUniform3fv(loc, 1, (const float*)data);    break;
			case ShaderUniformType::Vec4:  glUniform4fv(loc, 1, (const float*)data);    break;
			case ShaderUniformType::Mat3:
				glUniformMatrix3fv(loc, 1, GL_FALSE, (const float*)data); break;
			case ShaderUniformType::Mat4:
				glUniformMatrix4fv(loc, 1, GL_FALSE, (const float*)data); break;
			default: break;
			}
		}
	}

	void GLMaterial::Set(const std::string& name, const std::shared_ptr<TextureCube>& texture)
	{
		if (m_shader->GetSamplers().find(name) == m_shader->GetSamplers().end())
		{
			Log::Warn("GLMaterial::Set: sampler '{}' does not exist in '{}'", name, m_shader->GetName());
			return;
		}
		m_cubemapTextures[name] = texture;
	}

	void GLMaterial::Bind()
	{
		m_shader->Bind();
		UploadUniforms();

		for (const auto& [name, sampler] : m_shader->GetSamplers())
		{
			int32_t loc = m_shader->GetUniformLocation(name);
			if (loc == -1) continue;

			glUniform1i(loc, static_cast<int32_t>(sampler.slot));

			auto it2d = m_textures.find(name);
			if (it2d != m_textures.end() && it2d->second)
			{
				it2d->second->Bind(sampler.slot);
				continue;
			}

			auto itCube = m_cubemapTextures.find(name);
			if (itCube != m_cubemapTextures.end() && itCube->second)
				itCube->second->Bind(sampler.slot);
		}
	}
}