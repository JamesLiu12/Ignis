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

		static constexpr size_t DataTypeSize(DataType type) noexcept
		{
			switch (type)
			{
			case DataType::Float:  return sizeof(float);
			case DataType::Float2: return sizeof(float) * 2;
			case DataType::Float3: return sizeof(float) * 3;
			case DataType::Float4: return sizeof(float) * 4;

			case DataType::Bool:   return sizeof(bool);
			case DataType::Bool2:  return sizeof(bool) * 2;
			case DataType::Bool3:  return sizeof(bool) * 3;
			case DataType::Bool4:  return sizeof(bool) * 4;

			case DataType::Int:    return sizeof(int32_t);
			case DataType::Int2:   return sizeof(int32_t) * 2;
			case DataType::Int3:   return sizeof(int32_t) * 3;
			case DataType::Int4:   return sizeof(int32_t) * 4;

			case DataType::UInt:   return sizeof(uint32_t);
			case DataType::UInt2:  return sizeof(uint32_t) * 2;
			case DataType::UInt3:  return sizeof(uint32_t) * 3;
			case DataType::UInt4:  return sizeof(uint32_t) * 4;

			case DataType::Mat3:   return sizeof(float) * 3 * 3;
			case DataType::Mat4:   return sizeof(float) * 4 * 4;
			default:               return 0;
			}
		}

		static constexpr size_t DataTypeComponentCount(DataType type) noexcept
		{
			switch (type)
			{
			case DataType::Float:  return 1;
			case DataType::Float2: return 2;
			case DataType::Float3: return 3;
			case DataType::Float4: return 4;
			case DataType::Bool:   return 1;
			case DataType::Bool2:  return 2;
			case DataType::Bool3:  return 3;
			case DataType::Bool4:  return 4;
			case DataType::Int:    return 1;
			case DataType::Int2:   return 2;
			case DataType::Int3:   return 3;
			case DataType::Int4:   return 4;
			case DataType::UInt:   return 1;
			case DataType::UInt2:  return 2;
			case DataType::UInt3:  return 3;
			case DataType::UInt4:  return 4;
			case DataType::Mat3:   return 3;
			case DataType::Mat4:   return 4;
			default:               return 0;
			}
		}

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