#pragma once

#include "Texture.h"
#include "Ignis/Asset/Asset.h"
#include "Shader.h"

#include <glm/glm.hpp>

namespace ignis
{
	enum class MaterialType
	{
		Albedo,
		Normal,
		Metal,
		Roughness,
		Emissive,
		AO
	};

	struct MaterialData
	{
		AssetHandle AlbedoMap = AssetHandle::Invalid;
		AssetHandle NormalMap = AssetHandle::Invalid;
		AssetHandle MetalnessMap = AssetHandle::Invalid;
		AssetHandle RoughnessMap = AssetHandle::Invalid;
		AssetHandle EmissiveMap = AssetHandle::Invalid;
		AssetHandle AOMap = AssetHandle::Invalid;
	};

	class Material
	{
	public:
		virtual ~Material() = default;

		static std::shared_ptr<Material> Create(std::shared_ptr<Shader> shader, const std::string& name = "");
		static std::shared_ptr<Material> Create(const std::shared_ptr<Material>& other, const std::string& name = "");

		virtual void Set(const std::string& name, bool value) = 0;
		virtual void Set(const std::string& name, float value) = 0;
		virtual void Set(const std::string& name, const glm::vec2& value) = 0;
		virtual void Set(const std::string& name, const glm::vec3& value) = 0;
		virtual void Set(const std::string& name, const glm::vec4& value) = 0;
		virtual void Set(const std::string& name, int value) = 0;
		virtual void Set(const std::string& name, const glm::ivec2& value) = 0;
		virtual void Set(const std::string& name, const glm::ivec3& value) = 0;
		virtual void Set(const std::string& name, const glm::ivec4& value) = 0;
		virtual void Set(const std::string& name, uint32_t value) = 0;
		virtual void Set(const std::string& name, const glm::uvec2& value) = 0;
		virtual void Set(const std::string& name, const glm::uvec3& value) = 0;
		virtual void Set(const std::string& name, const glm::uvec4& value) = 0;
		virtual void Set(const std::string& name, const glm::mat3& value) = 0;
		virtual void Set(const std::string& name, const glm::mat4& value) = 0;
		virtual void Set(const std::string& name, const std::shared_ptr<Texture2D>& texture) = 0;
		virtual void Set(const std::string& name, const std::shared_ptr<TextureCube>& texture) = 0;

		virtual float& GetFloat(const std::string& name) = 0;
		virtual int32_t& GetInt(const std::string& name) = 0;
		virtual uint32_t& GetUInt(const std::string& name) = 0;
		virtual glm::vec2& GetVec2(const std::string& name) = 0;
		virtual glm::vec3& GetVec3(const std::string& name) = 0;
		virtual glm::vec4& GetVec4(const std::string& name) = 0;
		virtual glm::mat3& GetMat3(const std::string& name) = 0;
		virtual glm::mat4& GetMat4(const std::string& name) = 0;

		virtual void Bind() = 0;

		virtual std::shared_ptr<Shader> GetShader() const = 0;
		virtual const std::string& GetName() const = 0;
	};
}