#include "AssetManager.h"

namespace ignis
{
	static AssetType DetermineTypeFromExtension(const std::filesystem::path& path)
	{
		static const std::unordered_map<std::string, AssetType> extension_to_type = {
			{ ".png", AssetType::Texture },
			{ ".jpg", AssetType::Texture },
			{ ".jpeg", AssetType::Texture },
			{ ".obj", AssetType::Mesh },
			{ ".fbx", AssetType::Mesh },
		};

		auto it = extension_to_type.find(path.extension().string());
		if (it != extension_to_type.end())
		{
			return it->second;
		}

		return AssetType::Unknown;
	}

	bool AssetManager::IsAssetLoaded(AssetHandle handle)
	{
		return s_loaded_assets.find(handle) != s_loaded_assets.end();
	}

	AssetHandle AssetManager::ImportAsset(const std::filesystem::path& path)
	{
		AssetHandle handle = AssetHandle();
		AssetMetadata metadata;
		metadata.FilePath = path;
		metadata.Type = DetermineTypeFromExtension(path);

		s_asset_registry[handle] = metadata;

		return handle;
	}

	const AssetMetadata& AssetManager::GetMetadata(AssetHandle handle)
	{
		
		return s_asset_registry.at(handle);
	}
}