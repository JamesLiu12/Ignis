#pragma once

#include "AssetPackFile.h"
#include "Ignis/Core/API.h"
#include "Ignis/Scene/Scene.h"

namespace ignis
{
	class IGNIS_API AssetPack
	{
	public:
		AssetPack() = default;
		AssetPack(const std::filesystem::path& path);

		// Creation (Editor)
		static std::shared_ptr<AssetPack> CreateFromProject(
			const std::filesystem::path& project_path,
			std::atomic<float>& progress
		);

		// Loading (Runtime)
		static std::shared_ptr<AssetPack> Load(const std::filesystem::path& path);

		// Asset loading
		std::shared_ptr<Scene> LoadScene(AssetHandle scene_handle);
		std::shared_ptr<Asset> LoadAsset(AssetHandle scene_handle, AssetHandle asset_handle);

		// Script module
		std::vector<uint8_t> ReadScriptModule();

		// Metadata
		AssetHandle GetStartSceneHandle() const { return m_file.Index.StartSceneHandle; }
		uint64_t GetBuildVersion() const { return m_file.Header.BuildVersion; }
		bool IsAssetHandleValid(AssetHandle handle) const;
		const std::filesystem::path& GetPath() const { return m_path; }

	private:
		std::filesystem::path m_path;
		AssetPackFile m_file;
	};
}
