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
		AssetHandle NormalMap = AssetHandle::Invalid;
		AssetHandle MetalnessMap = AssetHandle::Invalid;
		AssetHandle RoughnessMap = AssetHandle::Invalid;
		AssetHandle EmissiveMap = AssetHandle::Invalid;
		AssetHandle AOMap = AssetHandle::Invalid;
	};
}