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
		AssetHandle AlbedoMap;
		AssetHandle NormalMap;
		AssetHandle MetalnessMap;
		AssetHandle RoughnessMap;
		AssetHandle EmissiveMap;
		AssetHandle AOMap;
	};

	class Material
	{
	public:
		virtual ~Material() = default;

		static std::shared_ptr<Material> Create(std::shared_ptr<Shader> shader, const std::string& name = "");
		static std::shared_ptr<Material> Create(std::shared_ptr<Material> other, const std::string& name = "");

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

		virtual std::shared_ptr<Shader> GetShader() const = 0;
		virtual const std::string& GetName() const = 0;
	};
}