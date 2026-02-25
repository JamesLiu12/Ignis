#pragma once

#include "Asset.h"

namespace ignis
{
	class AssetSerializer
	{
	public:
		bool Serialize(const std::unordered_map<AssetHandle, AssetMetadata>& asset_registry, const std::filesystem::path& filepath);
		std::optional<std::unordered_map<AssetHandle, AssetMetadata>> Deserialize(const std::filesystem::path& filepath);
	};
}