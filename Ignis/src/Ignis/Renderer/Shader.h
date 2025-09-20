#pragma once

#include <glm/glm.hpp>

namespace ignis
{
	class Shader
	{
	public:
		enum class DataType
		{
			Float, Float2, Float3, Float4,
			Bool, Bool2, Bool3, Bool4,
			Int, Int2, Int3, Int4,
			UInt, UInt2, UInt3, UInt4,
			Mat3, Mat4
		};

		virtual ~Shader() = default;

		virtual void Bind() = 0;
		virtual void UnBind() = 0;

		virtual void Set(const std::string& name, float value) = 0;
		virtual void Set(const std::string& name, const glm::vec2& vector) = 0;
		virtual void Set(const std::string& name, const glm::vec3& vector) = 0;
		virtual void Set(const std::string& name, const glm::vec4& vector) = 0;
		virtual void Set(const std::string& name, int value) = 0;
		virtual void Set(const std::string& name, const glm::ivec2& vector) = 0;
		virtual void Set(const std::string& name, const glm::ivec3& vector) = 0;
		virtual void Set(const std::string& name, const glm::ivec4& vector) = 0;
		virtual void Set(const std::string& name, unsigned int value) = 0;
		virtual void Set(const std::string& name, const glm::uvec2& vector) = 0;
		virtual void Set(const std::string& name, const glm::uvec3& vector) = 0;
		virtual void Set(const std::string& name, const glm::uvec4& vector) = 0;
		virtual void Set(const std::string& name, const glm::mat3& matrix) = 0;
		virtual void Set(const std::string& name, const glm::mat4& matrix) = 0;

		static std::shared_ptr<Shader> Create(const std::string& vertex_source, const std::string& fragment_source);
		static std::shared_ptr<Shader> CreateFromFile(const std::string& filepath);
	};
}