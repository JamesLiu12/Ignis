#pragma once

#include "Ignis/Renderer/Material.h"
#include "GLShader.h"

namespace ignis
{
	class GLMaterial : public Material
	{
	public:
		GLMaterial(std::shared_ptr<Shader> shader, const std::string& name = "");
		GLMaterial(const std::shared_ptr<Material>& other, const std::string& name = "");

		void Set(const std::string& name, bool value) override;
		void Set(const std::string& name, float value) override;
		void Set(const std::string& name, const glm::vec2& value) override;
		void Set(const std::string& name, const glm::vec3& value) override;
		void Set(const std::string& name, const glm::vec4& value) override;
		void Set(const std::string& name, int value) override;
		void Set(const std::string& name, const glm::ivec2& value) override;
		void Set(const std::string& name, const glm::ivec3& value) override;
		void Set(const std::string& name, const glm::ivec4& value) override;
		void Set(const std::string& name, uint32_t value) override;
		void Set(const std::string& name, const glm::uvec2& value) override;
		void Set(const std::string& name, const glm::uvec3& value) override;
		void Set(const std::string& name, const glm::uvec4& value) override;
		void Set(const std::string& name, const glm::mat3& value) override;
		void Set(const std::string& name, const glm::mat4& value) override;
		void Set(const std::string& name, const std::shared_ptr<Texture2D>& texture) override;
		void Set(const std::string& name, const std::shared_ptr<TextureCube>& texture) override;

		float& GetFloat(const std::string& name) override;
		int32_t& GetInt(const std::string& name) override;
		uint32_t& GetUInt(const std::string& name) override;
		glm::vec2& GetVec2(const std::string& name) override;
		glm::vec3& GetVec3(const std::string& name) override;
		glm::vec4& GetVec4(const std::string& name) override;
		glm::mat3& GetMat3(const std::string& name) override;
		glm::mat4& GetMat4(const std::string& name) override;

		void Bind() override;

		std::shared_ptr<Shader> GetShader() const override { return m_shader; }
		const std::string& GetName() const override { return m_name; }

	private:
		void SetBytes(const std::string& name, const void* data, uint32_t size);
		void* GetBytes(const std::string& name);

		template<typename T>
		T& GetValue(const std::string& name)
		{
			return *reinterpret_cast<T*>(GetBytes(name));
		}

		void UploadUniforms();

	private:
		std::shared_ptr<GLShader> m_shader;
		std::string m_name;

		std::vector<uint8_t>      m_uniform_buffer;

		std::unordered_map<std::string, std::shared_ptr<Texture2D>> m_textures;
		std::unordered_map<std::string, std::shared_ptr<TextureCube>> m_cubemap_textures;
	};
}