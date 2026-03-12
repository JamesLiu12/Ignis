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

	struct MaterialData
	{
		AssetHandle AlbedoMap = AssetHandle::Invalid;
		glm::vec4   AlbedoColor = glm::vec4(1.0f);
		uint32_t    AlbedoMapUVIndex = 0;

		AssetHandle NormalMap = AssetHandle::Invalid;
		uint32_t    NormalMapUVIndex = 0;

		AssetHandle MetalnessMap = AssetHandle::Invalid;
		float       MetallicValue = 0.0f;
		int         MetallicChannel = 0;
		uint32_t    MetalnessMapUVIndex = 0;

		AssetHandle RoughnessMap = AssetHandle::Invalid;
		float       RoughnessValue = 0.5f;
		int         RoughnessChannel = 0;
		uint32_t    RoughnessMapUVIndex = 0;

		AssetHandle EmissiveMap = AssetHandle::Invalid;
		glm::vec3   EmissiveColor = glm::vec3(0.0f);
		float       EmissiveIntensity = 1.0f;
		uint32_t    EmissiveMapUVIndex = 0;

		AssetHandle AOMap = AssetHandle::Invalid;
		uint32_t    AOMapUVIndex = 0;

		float ClearcoatFactor = 0.0f;
		float ClearcoatRoughnessFactor = 0.0f;
		AssetHandle ClearcoatMap = AssetHandle::Invalid;
		uint32_t    ClearcoatMapUVIndex = 0;
		AssetHandle ClearcoatRoughnessMap = AssetHandle::Invalid;
		uint32_t    ClearcoatRoughnessMapUVIndex = 0;
		AssetHandle ClearcoatNormalMap = AssetHandle::Invalid;
		uint32_t    ClearcoatNormalMapUVIndex = 0;
	};
}