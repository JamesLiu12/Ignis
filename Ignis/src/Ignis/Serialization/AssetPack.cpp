#include "AssetPack.h"
#include "AssetPackSerializer.h"
#include "FileStream.h"
#include "Ignis/Core/Log.h"
#include "Ignis/Scene/SceneSerializer.h"
#include "Ignis/Project/Project.h"
#include "Ignis/Asset/AssetManager.h"

namespace ignis
{
	AssetPack::AssetPack(const std::filesystem::path& path)
		: m_path(path)
	{
	}

	std::shared_ptr<AssetPack> AssetPack::CreateFromProject(
		const std::filesystem::path& project_path,
		std::atomic<float>& progress
	)
	{
		auto pack = std::make_shared<AssetPack>();
		pack->m_path = project_path / "Build" / "AssetPack.igpack";

		Log::CoreInfo("Creating asset pack at: {}", pack->m_path.string());

		// Get current timestamp for build version
		auto now = std::chrono::system_clock::now();
		auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
		pack->m_file.Header.BuildVersion = timestamp;

		// Load project
		auto project = Project::GetActive();
		if (!project)
		{
			Log::CoreError("No active project for asset pack creation");
			return nullptr;
		}

		// Find all scene files in asset directory
		std::vector<std::filesystem::path> scene_paths;
		auto asset_dir = project->GetAssetDirectory();
		
		if (std::filesystem::exists(asset_dir))
		{
			for (const auto& entry : std::filesystem::recursive_directory_iterator(asset_dir))
			{
				if (entry.is_regular_file() && entry.path().extension() == ".scene")
				{
					scene_paths.push_back(entry.path());
				}
			}
		}

		if (scene_paths.empty())
		{
			Log::CoreWarn("No scenes found in project for asset pack");
		}

		// Set start scene handle - generate UUID from start scene path
		auto start_scene_path = project->GetConfig().StartScene;
		if (!start_scene_path.empty())
		{
			auto full_start_scene_path = asset_dir / start_scene_path;
			// Generate deterministic UUID from path
			pack->m_file.Index.StartSceneHandle = AssetHandle(full_start_scene_path.string());
		}

		if (pack->m_file.Index.StartSceneHandle == AssetHandle::Invalid && !scene_paths.empty())
		{
			Log::CoreWarn("Start scene not found, using first scene as default");
			pack->m_file.Index.StartSceneHandle = AssetHandle(scene_paths[0].string());
		}

		// Create output directory
		std::filesystem::create_directories(pack->m_path.parent_path());

		// Open file stream
		FileStreamWriter writer(pack->m_path);
		if (!writer.IsStreamGood())
		{
			Log::CoreError("Failed to create asset pack file");
			return nullptr;
		}

		// Write header
		writer.WriteData(pack->m_file.Header.HEADER, 4);
		writer.WriteRaw(pack->m_file.Header.Version);
		writer.WriteRaw(pack->m_file.Header.BuildVersion);

		// Reserve space for index table (will write later)
		uint64_t index_position = writer.GetStreamPosition();
		uint64_t estimated_index_size = 1024 * 1024; // 1MB estimate, will recalculate
		writer.WriteZero(estimated_index_size);

		// Serialize all scenes and their assets
		// Note: Full implementation will be done when asset importers are extended
		// For now, we'll serialize scenes as JSON strings
		float progress_per_scene = scene_paths.empty() ? 0.0f : 0.8f / scene_paths.size();
		for (const auto& scene_path : scene_paths)
		{
			// Generate handle from path
			AssetHandle scene_handle = AssetHandle(scene_path.string());
			Log::CoreInfo("Packing scene: {} ({})", scene_path.filename().string(), scene_handle.ToString());

			// Read scene file directly
			std::ifstream scene_file(scene_path, std::ios::binary);
			if (!scene_file.is_open())
			{
				Log::CoreWarn("Failed to open scene file: {}", scene_path.string());
				continue;
			}

			std::string scene_data((std::istreambuf_iterator<char>(scene_file)), std::istreambuf_iterator<char>());
			scene_file.close();

			// Serialize scene
			AssetPackFile::SceneInfo scene_info;
			scene_info.PackedOffset = writer.GetStreamPosition();

			writer.WriteString(scene_data);
			scene_info.PackedSize = writer.GetStreamPosition() - scene_info.PackedOffset;

			// Store scene info
			pack->m_file.Index.Scenes[scene_handle] = scene_info;

			// TODO: Collect and serialize referenced assets
			// This will be implemented when asset importers are extended with binary serialization

			progress += progress_per_scene;
		}

		// Serialize script module
		auto script_module_path = project->ResolveScriptModulePath();
		if (!script_module_path.empty() && std::filesystem::exists(script_module_path))
		{
			Log::CoreInfo("Packing script module: {}", script_module_path.string());

			std::ifstream script_file(script_module_path, std::ios::binary | std::ios::ate);
			if (script_file.is_open())
			{
				size_t file_size = script_file.tellg();
				script_file.seekg(0, std::ios::beg);

				std::vector<uint8_t> script_data(file_size);
				script_file.read((char*)script_data.data(), file_size);
				script_file.close();

				pack->m_file.Index.PackedScriptModuleOffset = writer.GetStreamPosition();
				pack->m_file.Index.PackedScriptModuleSize = file_size;
				writer.WriteBuffer(script_data.data(), file_size, false);

				Log::CoreInfo("Script module packed: {} bytes", file_size);
			}
		}

		// Write index table at reserved position
		uint64_t end_position = writer.GetStreamPosition();
		writer.SetStreamPosition(index_position);

		AssetPackSerializer::SerializeIndexTable(writer, pack->m_file.Index);
		uint64_t actual_index_size = writer.GetStreamPosition() - index_position;

		// Verify we didn't overflow the reserved space
		if (actual_index_size > estimated_index_size)
		{
			Log::CoreError("Index table size exceeded estimate! {} > {}", actual_index_size, estimated_index_size);
		}

		progress = 1.0f;
		Log::CoreInfo("Asset pack created successfully: {}", pack->m_path.string());

		return pack;
	}

	std::shared_ptr<AssetPack> AssetPack::Load(const std::filesystem::path& path)
	{
		auto pack = std::make_shared<AssetPack>(path);

		if (!AssetPackSerializer::DeserializeIndex(path, pack->m_file))
		{
			Log::CoreError("Failed to load asset pack: {}", path.string());
			return nullptr;
		}

		Log::CoreInfo("Asset pack loaded: {}", path.string());
		Log::CoreInfo("  Build version: {}", pack->m_file.Header.BuildVersion);
		Log::CoreInfo("  Start scene: {}", pack->m_file.Index.StartSceneHandle.ToString());
		Log::CoreInfo("  Scenes: {}", pack->m_file.Index.Scenes.size());

		return pack;
	}

	std::shared_ptr<Scene> AssetPack::LoadScene(AssetHandle scene_handle)
	{
		auto it = m_file.Index.Scenes.find(scene_handle);
		if (it == m_file.Index.Scenes.end())
		{
			Log::CoreError("Scene not found in asset pack: {}", scene_handle.ToString());
			return nullptr;
		}

		const auto& scene_info = it->second;

		FileStreamReader reader(m_path);
		if (!reader.IsStreamGood())
		{
			Log::CoreError("Failed to open asset pack for reading");
			return nullptr;
		}

		reader.SetStreamPosition(scene_info.PackedOffset);
		std::string scene_data = reader.ReadString();

		// Write to temp file and deserialize
		std::filesystem::path temp_path = std::filesystem::temp_directory_path() / "temp_scene_load.json";
		std::ofstream temp_file(temp_path);
		temp_file << scene_data;
		temp_file.close();

		SceneSerializer serializer;
		auto scene = serializer.Deserialize(temp_path);
		std::filesystem::remove(temp_path);

		if (scene)
		{
			Log::CoreInfo("Scene loaded from pack: {}", scene_handle.ToString());
		}

		return scene;
	}

	std::shared_ptr<Asset> AssetPack::LoadAsset(AssetHandle scene_handle, AssetHandle asset_handle)
	{
		// Find scene
		auto scene_it = m_file.Index.Scenes.find(scene_handle);
		if (scene_it == m_file.Index.Scenes.end())
		{
			Log::CoreError("Scene not found in asset pack: {}", scene_handle.ToString());
			return nullptr;
		}

		// Find asset in scene
		auto& scene_info = scene_it->second;
		auto asset_it = scene_info.Assets.find(asset_handle);
		if (asset_it == scene_info.Assets.end())
		{
			Log::CoreError("Asset not found in scene: {}", asset_handle.ToString());
			return nullptr;
		}

		const auto& asset_info = asset_it->second;

		// TODO: Implement asset deserialization when asset importers are extended
		// For now, return nullptr
		Log::CoreWarn("Asset loading from pack not yet implemented: {}", asset_handle.ToString());

		return nullptr;
	}

	std::vector<uint8_t> AssetPack::ReadScriptModule()
	{
		if (m_file.Index.PackedScriptModuleSize == 0)
		{
			Log::CoreWarn("No script module in asset pack");
			return {};
		}

		FileStreamReader reader(m_path);
		if (!reader.IsStreamGood())
		{
			Log::CoreError("Failed to open asset pack for reading script module");
			return {};
		}

		reader.SetStreamPosition(m_file.Index.PackedScriptModuleOffset);

		std::vector<uint8_t> script_data(m_file.Index.PackedScriptModuleSize);
		reader.ReadBuffer(script_data.data(), m_file.Index.PackedScriptModuleSize);

		Log::CoreInfo("Script module read from pack: {} bytes", script_data.size());

		return script_data;
	}

	bool AssetPack::IsAssetHandleValid(AssetHandle handle) const
	{
		// Check if handle exists in any scene
		for (const auto& [scene_handle, scene_info] : m_file.Index.Scenes)
		{
			if (scene_handle == handle)
				return true;

			if (scene_info.Assets.find(handle) != scene_info.Assets.end())
				return true;
		}

		return false;
	}
}
