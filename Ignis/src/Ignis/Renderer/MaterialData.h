#pragma once

#include "Ignis/Asset/Asset.h"

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

	struct UVTransform
	{
		glm::vec2 Offset = { 0.0f, 0.0f };
		glm::vec2 Scale = { 1.0f, 1.0f };
		float     Rotation = 0.0f;

		glm::mat3 ToMatrix() const
		{
			float c = std::cos(Rotation);
			float s = std::sin(Rotation);
			return glm::mat3(
				glm::vec3(c * Scale.x, -s * Scale.x, 0.0f),
				glm::vec3(s * Scale.y, c * Scale.y, 0.0f),
				glm::vec3(Offset.x, Offset.y, 1.0f)
			);
		}
	};

	struct MaterialData
	{
		AssetHandle AlbedoMap = AssetHandle::Invalid;
		glm::vec4   AlbedoColor = glm::vec4(1.0f);
		uint32_t    AlbedoMapUVIndex = 0;
		UVTransform AlbedoMapUVTransform;

		AssetHandle NormalMap = AssetHandle::Invalid;
		uint32_t    NormalMapUVIndex = 0;
		UVTransform NormalMapUVTransform;

		AssetHandle MetalnessMap = AssetHandle::Invalid;
		float       MetallicValue = 0.0f;
		int         MetallicChannel = 0;
		uint32_t    MetalnessMapUVIndex = 0;
		UVTransform MetalnessMapUVTransform;

		AssetHandle RoughnessMap = AssetHandle::Invalid;
		float       RoughnessValue = 0.5f;
		int         RoughnessChannel = 0;
		uint32_t    RoughnessMapUVIndex = 0;
		UVTransform RoughnessMapUVTransform;

		AssetHandle EmissiveMap = AssetHandle::Invalid;
		glm::vec3   EmissiveColor = glm::vec3(0.0f);
		float       EmissiveIntensity = 1.0f;
		uint32_t    EmissiveMapUVIndex = 0;
		UVTransform EmissiveMapUVTransform;

		AssetHandle AOMap = AssetHandle::Invalid;
		uint32_t    AOMapUVIndex = 0;
		UVTransform AOMapUVTransform;

		float ClearcoatFactor = 0.0f;
		float ClearcoatRoughnessFactor = 0.0f;

		AssetHandle ClearcoatMap = AssetHandle::Invalid;
		uint32_t    ClearcoatMapUVIndex = 0;
		UVTransform ClearcoatMapUVTransform;

		AssetHandle ClearcoatRoughnessMap = AssetHandle::Invalid;
		uint32_t    ClearcoatRoughnessMapUVIndex = 0;
		UVTransform ClearcoatRoughnessMapUVTransform;

		AssetHandle ClearcoatNormalMap = AssetHandle::Invalid;
		uint32_t    ClearcoatNormalMapUVIndex = 0;
		UVTransform ClearcoatNormalMapUVTransform;

		bool DoubleSided = false;
	};
}