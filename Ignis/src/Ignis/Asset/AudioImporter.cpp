#include "AudioImporter.h"
#include "Ignis/Audio/AudioClip.h"
#include "AssetLoadContext.h"

#include <filesystem>
#include <algorithm>
#include <unordered_set>

namespace ignis
{
	AssetType AudioImporter::GetType() const { return AssetType::AudioClip; }

	std::shared_ptr<Asset> AudioImporter::Import(const AssetMetadata& metadata, const AssetLoadContext& context)
	{
		const auto* opts = std::get_if<AudioImportOptions>(&metadata.ImportOptions);
		const AudioImportOptions& options = opts ? *opts : AudioImportOptions{};

		std::filesystem::path resolved = VFS::Resolve(metadata.FilePath.string());
		if (!std::filesystem::exists(resolved))
		{
			Log::CoreError("AudioImporter: File not found '{}'", metadata.FilePath);
			return nullptr;
		}

		// Validate extension
		std::string ext = resolved.extension().string();
		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

		static const std::unordered_set<std::string> k_supported = { ".wav", ".mp3", ".flac", ".ogg" };
		if (!k_supported.count(ext))
		{
			Log::CoreError("AudioImporter: Unsupported format '{}' for '{}'", ext, metadata.FilePath);
			return nullptr;
		}

		auto clip = std::make_shared<AudioClip>();
		clip->m_filepath = resolved.string();
		clip->m_stream = options.Stream;

		Log::CoreInfo("AudioImporter: Registered '{}' ({})",
			metadata.FilePath, clip->m_stream ? "streaming" : "in-memory");
		return clip;
	}

	AudioImporter& AudioImporter::Get()
	{
		static AudioImporter instance;
		return instance;
	}
}