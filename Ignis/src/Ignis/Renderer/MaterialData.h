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

		AssetHandle NormalMap = AssetHandle::Invalid;

		AssetHandle MetalnessMap = AssetHandle::Invalid;
		float       MetallicValue = 0.0f;

		AssetHandle RoughnessMap = AssetHandle::Invalid;
		float       RoughnessValue = 0.5f;

		AssetHandle EmissiveMap = AssetHandle::Invalid;
		glm::vec3   EmissiveColor = glm::vec3(0.0f);
		float       EmissiveIntensity = 1.0f;

		AssetHandle AOMap = AssetHandle::Invalid;
	};
}