#pragma once

#include "Asset.h"
#include "VFS.h"

namespace ignis
{
	struct AssetMetadata
	{
		AssetHandle handle = AssetHandle::Invalid;
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
			
			// Check memory-only assets
			if (IsMemoryAsset(handle))
			{
				return std::static_pointer_cast<T>(s_memory_assets.at(handle));
			}

			const AssetMetadata* metadata = GetMetadata(handle);

			if (!metadata)
			{
				return nullptr;
			}

			std::shared_ptr<Asset> asset = LoadAssetFromFile(*metadata);

			if (asset)
			{
				asset->m_handle = handle;
				s_loaded_assets[handle] = asset;
				return std::static_pointer_cast<T>(asset);
			}

			return nullptr;
		}

		// Add memory-only asset (for default textures, procedural meshes, etc.)
		template<typename T>
			requires std::derived_from<T, Asset>
		static AssetHandle AddMemoryOnlyAsset(std::shared_ptr<T> asset)
		{
			AssetHandle handle = AssetHandle();
			asset->m_handle = handle;
			s_memory_assets[handle] = asset;
			return handle;
		}

		static bool IsMemoryAsset(AssetHandle handle);
		static bool IsAssetLoaded(AssetHandle handle);

		static AssetHandle ImportAsset(const std::filesystem::path& path, AssetType asset_type = AssetType::Unknown);
		static void RemoveAsset(AssetHandle handle);

		static const AssetMetadata* GetMetadata(AssetHandle handle);
		static const AssetMetadata* GetMetadata(std::filesystem::path path);

	private:
		static std::shared_ptr<Asset> LoadAssetFromFile(const AssetMetadata& metadata);

		inline static std::unordered_map<AssetHandle, std::shared_ptr<Asset>> s_loaded_assets;
		inline static std::unordered_map<AssetHandle, std::shared_ptr<Asset>> s_memory_assets;
		inline static std::unordered_map<AssetHandle, AssetMetadata> s_asset_registry;
	};
}