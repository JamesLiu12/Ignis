#pragma once

#include "Ignis/Core/API.h"
#include "Asset.h"
#include "AssetImporter.h"

namespace ignis
{
	class AssetPack;
	class Scene;

	enum class AssetLoadMode
	{
		Editor,   // Load from filesystem via asset registry
		Runtime   // Load from asset pack
	};
	class IGNIS_API AssetManager
	{
	public:
		template<std::derived_from<Asset> T>
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

			std::shared_ptr<Asset> asset = nullptr;

			// Load based on mode
			if (s_load_mode == AssetLoadMode::Runtime)
			{
				// Runtime: Load from asset pack
				asset = LoadAssetFromPack(handle);
			}
			else
			{
				// Editor: Load from filesystem
				const AssetMetadata* metadata = GetMetadata(handle);
				if (!metadata)
				{
					return nullptr;
				}
				
				asset = LoadAssetFromFile(*metadata);
			}

			if (asset)
			{
				asset->m_handle = handle;
				s_loaded_assets[handle] = asset;
				return std::static_pointer_cast<T>(asset);
			}

			return nullptr;
		}

		// Add memory-only asset (for default textures, procedural meshes, etc.)
		template<std::derived_from<Asset> T>
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
		static void UnloadAsset(AssetHandle handle);

		static const AssetMetadata* GetMetadata(AssetHandle handle);
		static const AssetMetadata* GetMetadata(std::filesystem::path path);
		static AssetMetadata* GetMetadataMutable(AssetHandle handle);

		static bool LoadAssetRegistry(const std::filesystem::path& path);
		static bool SaveAssetRegistry(const std::filesystem::path& path);

		static const std::unordered_map<AssetHandle, AssetMetadata>& GetAssetRegistry() { return s_asset_registry; }

		static void SetLoadContext(const AssetLoadContext& context);
		static AssetLoadContext& GetLoadContext();

		static AssetType DetermineTypeFromExtension(const std::filesystem::path& path);

		// Runtime mode control
		static void SetLoadMode(AssetLoadMode mode);
		static AssetLoadMode GetLoadMode() { return s_load_mode; }

		static void SetAssetPack(std::shared_ptr<AssetPack> pack);
		static std::shared_ptr<AssetPack> GetAssetPack() { return s_asset_pack; }

		static void SetActiveScene(AssetHandle scene_handle);
		static AssetHandle GetActiveScene() { return s_active_scene; }

		// Scene loading (runtime only)
		static std::shared_ptr<Scene> LoadScene(AssetHandle scene_handle);

		// Validate asset handle (mode-aware)
		static bool IsAssetHandleValid(AssetHandle handle);

	private:
		static std::shared_ptr<Asset> LoadAssetFromFile(const AssetMetadata& metadata);
		static std::shared_ptr<Asset> LoadAssetFromPack(AssetHandle handle);
		static AssetImportOptions     DefaultImportOptions(AssetType type);

		inline static std::unordered_map<AssetHandle, std::shared_ptr<Asset>> s_loaded_assets;
		inline static std::unordered_map<AssetHandle, std::shared_ptr<Asset>> s_memory_assets;
		inline static std::unordered_map<AssetHandle, AssetMetadata> s_asset_registry;
		inline static AssetLoadContext s_load_context;

		// Runtime mode state
		inline static AssetLoadMode s_load_mode = AssetLoadMode::Editor;
		inline static std::shared_ptr<AssetPack> s_asset_pack = nullptr;
		inline static AssetHandle s_active_scene = AssetHandle::Invalid;
	};
}