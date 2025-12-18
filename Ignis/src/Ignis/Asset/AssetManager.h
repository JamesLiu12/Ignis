#pragma once

#include "Asset.h"
#include "VFS.h"
#include "MeshImporter.h"

namespace ignis
{
	struct AssetMetadata
	{
		AssetType Type;
		std::filesystem::path FilePath;
	};

	class AssetManager
	{
	public:
		template<typename T>
			requires std::derived_from<T, Asset>
		static std::shared_ptr<T> GetAsset(AssetHandle handle)
		{
			if (IsAssetLoaded(handle))
			{
				return std::static_pointer_cast<T>(s_loaded_assets.at(handle));
			}

			const AssetMetadata& metadata = GetMetadata(handle);

			std::shared_ptr<T> asset = LoadAssetFromFile<T>(metadata);

			if (asset)
			{
				s_loaded_assets[handle] = asset;
			}
			return asset;
		}

		static bool IsAssetLoaded(AssetHandle handle);

		static AssetHandle ImportAsset(const std::filesystem::path& path);

	private:
		template<typename T>
			requires std::derived_from<T, Asset>
		static std::shared_ptr<T> LoadAssetFromFile(const AssetMetadata& metadata)
		{
			if (!VFS::Exists(metadata.FilePath.string()))
			{
				Log::CoreError("Asset file does not exist: {}", metadata.FilePath.string());
				return nullptr;
			}

			switch (metadata.Type)
			{
			case AssetType::Texture:
			{
				// TODO AssetImporter
				return nullptr;
			}
			case AssetType::Mesh:
			{
				return MeshImporter::ImportMesh(metadata.FilePath.string());
			}
			case AssetType::Unknown:
			{
				Log::CoreError("Unknown asset type for file: {}", metadata.FilePath.string());
				return nullptr;
			}
			default:
				Log::CoreError("Unsupported asset type for file: {}", metadata.FilePath.string());
				return nullptr;
			}
		}

		static const AssetMetadata& GetMetadata(AssetHandle handle);

		inline static std::unordered_map<AssetHandle, std::shared_ptr<Asset>> s_loaded_assets;
		inline static std::unordered_map<AssetHandle, AssetMetadata> s_asset_registry;
	};
}