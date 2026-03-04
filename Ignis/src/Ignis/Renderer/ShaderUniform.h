#pragma once
#include <string>
#include <cstdint>

namespace ignis {

	enum class ShaderUniformType
	{
		None = 0,
		Bool,
		Float, Vec2, Vec3, Vec4,
		Int, IVec2, IVec3, IVec4,
		UInt, UVec2, UVec3, UVec4,
		Mat3, Mat4
	};

	struct ShaderUniform
	{
		std::string       name;
		ShaderUniformType type = ShaderUniformType::None;
		uint32_t          size = 0;
		uint32_t          offset = 0;

		static uint32_t TypeSize(ShaderUniformType type)
		{
			switch (type)
			{
			case ShaderUniformType::Bool:  return sizeof(int32_t);
			case ShaderUniformType::Float: return sizeof(float);
			case ShaderUniformType::Vec2:  return sizeof(float) * 2;
			case ShaderUniformType::Vec3:  return sizeof(float) * 3;
			case ShaderUniformType::Vec4:  return sizeof(float) * 4;
			case ShaderUniformType::Int:   return sizeof(int32_t);
			case ShaderUniformType::IVec2: return sizeof(int32_t) * 2;
			case ShaderUniformType::IVec3: return sizeof(int32_t) * 3;
			case ShaderUniformType::IVec4: return sizeof(int32_t) * 4;
			case ShaderUniformType::UInt:  return sizeof(uint32_t);
			case ShaderUniformType::UVec2: return sizeof(uint32_t) * 2;
			case ShaderUniformType::UVec3: return sizeof(uint32_t) * 3;
			case ShaderUniformType::UVec4: return sizeof(uint32_t) * 4;
			case ShaderUniformType::Mat3:  return sizeof(float) * 9;
			case ShaderUniformType::Mat4:  return sizeof(float) * 16;
			default:                       return 0;
			}
		}
	};

	struct ShaderSampler
	{
		std::string name;
		uint32_t    slot = 0;
	};

}