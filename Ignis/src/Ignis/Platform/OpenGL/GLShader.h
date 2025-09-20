#pragma once

#include "Ignis/Renderer/Shader.h"

namespace ignis
{
	class GLShader : public Shader
	{
	public:
		GLShader(const std::string& vertex_source, const std::string& fragment_source);
		GLShader(const std::string& filepath);

		~GLShader() override;

		void Bind();
		void UnBind();

		void Set(const std::string& name, float value);
		void Set(const std::string& name, const glm::vec2& vector);
		void Set(const std::string& name, const glm::vec3& vector);
		void Set(const std::string& name, const glm::vec4& vector);
		void Set(const std::string& name, int value);
		void Set(const std::string& name, const glm::ivec2& vector);
		void Set(const std::string& name, const glm::ivec3& vector);
		void Set(const std::string& name, const glm::ivec4& vector);
		void Set(const std::string& name, unsigned int value);
		void Set(const std::string& name, const glm::uvec2& vector);
		void Set(const std::string& name, const glm::uvec3& vector);
		void Set(const std::string& name, const glm::uvec4& vector);
		void Set(const std::string& name, const glm::mat3& matrix);
		void Set(const std::string& name, const glm::mat4& matrix);

	private:
		unsigned int m_id = 0;
	};
}