#include "AssetManager.h"
#include "MeshImporter.h"
#include "TextureImporter.h"
#include "FontImporter.h"
#include "AssetSerializer.h"
#include "AudioImporter.h"

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
			{ ".png", AssetType::Texture2D },
			{ ".jpg", AssetType::Texture2D },
			{ ".jpeg", AssetType::Texture2D },
			{ ".tga", AssetType::Texture2D},
			{ ".hdr", AssetType::EquirectIBLEnv},
			{ ".obj", AssetType::Mesh },
			{ ".fbx", AssetType::Mesh },
			{ ".ttf", AssetType::Font },
			{ ".wav", AssetType::AudioClip },
			{ ".mp3", AssetType::AudioClip },
			{ ".flac", AssetType::AudioClip },
			{ ".ogg", AssetType::AudioClip },
		};

		std::string extension = ToLowerASCII(path.extension().string());

		auto it = extension_to_type.find(extension);
		if (it != extension_to_type.end())
		{
			return it->second;
		}

		return AssetType::Unknown;
	}

	AssetImportOptions AssetManager::DefaultImportOptions(AssetType type)
	{
		switch (type)
		{
		case AssetType::Texture2D:
			return TextureImportOptions{};

		case AssetType::TextureCube:
		{
			TextureImportOptions opts{};
			opts.FlipVertical = false;
			opts.WrapS = TextureWrap::ClampToEdge;
			opts.WrapT = TextureWrap::ClampToEdge;
			return opts;
		}

		case AssetType::EquirectIBLEnv:
		{
			EquirectImportOptions opts{};
			opts.TexOptions.FlipVertical = false;
			opts.TexOptions.GenMipmaps = false;
			return opts;
		}

		case AssetType::Font:
			return FontImportOptions{};

		case AssetType::AudioClip:
			return AudioImportOptions{};

		default:
			return std::monostate{};
		}
	}

	bool AssetManager::IsAssetLoaded(AssetHandle handle)
	{
		return s_loaded_assets.find(handle) != s_loaded_assets.end();
	}

	bool AssetManager::IsMemoryAsset(AssetHandle handle)
	{
		return s_memory_assets.find(handle) != s_memory_assets.end();
	}

	AssetHandle AssetManager::ImportAsset(const std::filesystem::path& path, AssetType asset_type)
	{
		std::string vfs_path = VFS::ToVFSPath(path);
		const AssetMetadata* existing = GetMetadata(vfs_path);
		if (existing)
			return existing->Handle;

		AssetHandle handle = AssetHandle();

		AssetMetadata metadata;
		metadata.Handle = handle;
		metadata.FilePath = vfs_path;
		metadata.Type = (asset_type == AssetType::Unknown)
			? DetermineTypeFromExtension(path)
			: asset_type;
		metadata.ImportOptions = DefaultImportOptions(metadata.Type);

		s_asset_registry[handle] = metadata;
		return handle;
	}

	void AssetManager::RemoveAsset(AssetHandle handle)
	{
		s_loaded_assets.erase(handle);
		s_asset_registry.erase(handle);
	}

	void AssetManager::UnloadAsset(AssetHandle handle)
	{
		s_loaded_assets.erase(handle);
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

	AssetMetadata* AssetManager::GetMetadataMutable(AssetHandle handle)
	{
		auto it = s_asset_registry.find(handle);
		return (it != s_asset_registry.end()) ? &it->second : nullptr;
	}


	bool AssetManager::LoadAssetRegistry(const std::filesystem::path& path)
	{
		AssetSerializer asset_serializer;
		if (auto registry = asset_serializer.Deserialize(path))
		{
			s_asset_registry = registry.value();
			return true;
		}
		return false;
	}

	bool AssetManager::SaveAssetRegistry(const std::filesystem::path& path)
	{
		AssetSerializer asset_serializer;
		return asset_serializer.Serialize(s_asset_registry, path);
	}

	void AssetManager::SetLoadContext(const AssetLoadContext& context)
	{
		s_load_context = context;
	}

	AssetLoadContext& AssetManager::GetLoadContext()
	{
		return s_load_context;
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
		case AssetType::Texture2D:
			return Texture2DImporter::Get().Import(metadata, s_load_context);
		case AssetType::TextureCube:
			return TextureCubeImporter::Get().Import(metadata, s_load_context);
		case AssetType::EquirectIBLEnv:
			return EquirectEnvImporter::Get().Import(metadata, s_load_context);
		case AssetType::Mesh:
			return MeshImporter::Get().Import(metadata, s_load_context);
		case AssetType::Font:
			return FontImporter::Get().Import(metadata, s_load_context);
		case AssetType::AudioClip:
			return AudioImporter::Get().Import(metadata, s_load_context);
		default:
			Log::CoreError("Unknown asset type for file: {}", metadata.FilePath.string());
			return nullptr;
		}
	}
}