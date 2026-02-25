#pragma once

#include "Asset.h"

namespace ignis
{
	class AssetSerializer
	{
	public:
		bool Serialize(const std::unordered_map<AssetHandle, AssetMetadata>& asset_registry, std::filesystem::path filepath);
		std::unordered_map<AssetHandle, AssetMetadata> Deserialize(std::filesystem::path filepath);
	};
}