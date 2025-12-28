#include "AssetManager.h"
#include "MeshImporter.h"
#include "TextureImporter.h"

namespace ignis
{
	static std::string ToLowerASCII(std::string s)
	{
		std::transform(s.begin(), s.end(), s.begin(),
			[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
		return s;
	}

	static AssetType DetermineTypeFromExtension(const std::filesystem::path& path)
	{
		static const std::unordered_map<std::string, AssetType> extension_to_type = {
			{ ".png", AssetType::Texture },
			{ ".jpg", AssetType::Texture },
			{ ".jpeg", AssetType::Texture },
			{ ".tga", AssetType::Texture},
			{ ".obj", AssetType::Mesh },
			{ ".fbx", AssetType::Mesh },
		};

		std::string extension = ToLowerASCII(path.extension().string());

		auto it = extension_to_type.find(extension);
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
		const AssetMetadata* metadata = GetMetadata(path);

		if (metadata)
		{
			return metadata->handle;
		}
		
		AssetHandle handle = AssetHandle();
		AssetMetadata new_metadata;
		new_metadata.FilePath = path;
		new_metadata.Type = DetermineTypeFromExtension(path);
		new_metadata.handle = handle;

		s_asset_registry[handle] = new_metadata;

		return handle;
	}

	void AssetManager::RemoveAsset(AssetHandle handle)
	{
		s_loaded_assets.erase(handle);
		s_asset_registry.erase(handle);
	}

	const AssetMetadata* AssetManager::GetMetadata(AssetHandle handle)
	{
		auto it = s_asset_registry.find(handle);
		if (it != s_asset_registry.end()) {
			return &it->second;
		}
		return nullptr;
	}

	const AssetMetadata* AssetManager::GetMetadata(std::filesystem::path path)
	{
		for (const auto& [handle, metadata] : s_asset_registry)
		{
			if (metadata.FilePath == path)
			{
				return &metadata;
			}
		}

		return nullptr;
	}

	std::shared_ptr<Asset> AssetManager::LoadAssetFromFile(const AssetMetadata& metadata)
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
			 return TextureImporter::ImportTexture2D(metadata.FilePath.string());
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
		}
		return nullptr;
	}
}