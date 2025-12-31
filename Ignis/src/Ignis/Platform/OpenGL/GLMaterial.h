#pragma once

#include "Ignis/Renderer/Material.h"
#include "GLShader.h"

namespace ignis
{
	class GLMaterial : public Material
	{
	public:
		GLMaterial(std::shared_ptr<Shader> shader, const std::string& name = "");
		GLMaterial(std::shared_ptr<Material> other, const std::string& name = "");

		void Set(const std::string& name, float value) override;
		void Set(const std::string& name, const glm::vec2& vector) override;
		void Set(const std::string& name, const glm::vec3& vector) override;
		void Set(const std::string& name, const glm::vec4& vector) override;
		void Set(const std::string& name, int value) override;
		void Set(const std::string& name, const glm::ivec2& vector) override;
		void Set(const std::string& name, const glm::ivec3& vector) override;
		void Set(const std::string& name, const glm::ivec4& vector) override;
		void Set(const std::string& name, unsigned int value) override;
		void Set(const std::string& name, const glm::uvec2& vector) override;
		void Set(const std::string& name, const glm::uvec3& vector) override;
		void Set(const std::string& name, const glm::uvec4& vector) override;
		void Set(const std::string& name, const glm::mat3& matrix) override;
		void Set(const std::string& name, const glm::mat4& matrix) override;

		std::shared_ptr<Shader> GetShader() const override { return m_shader; }
		const std::string& GetName() const override { return m_name; }

	private:
		std::shared_ptr<GLShader> m_shader;
		std::string m_name;
	};
}