#include "AssetPackSerializer.h"
#include "Ignis/Core/Log.h"
#include "Ignis/Core/UUID.h"

namespace ignis
{
	void AssetPackSerializer::Serialize(
		const std::filesystem::path& path,
		AssetPackFile& file,
		const std::vector<uint8_t>& script_module,
		std::atomic<float>& progress
	)
	{
		FileStreamWriter writer(path);
		if (!writer.IsStreamGood())
		{
			Log::CoreError("Failed to create asset pack file: {}", path.string());
			return;
		}

		// Write header
		writer.WriteData(file.Header.HEADER, 4);
		writer.WriteRaw(file.Header.Version);
		writer.WriteRaw(file.Header.BuildVersion);

		// Reserve space for index table (will write later)
		uint64_t index_position = writer.GetStreamPosition();
		uint64_t index_size = CalculateIndexTableSize(file);
		writer.WriteZero(index_size);

		// Note: Scene and asset data will be written by AssetPack::CreateFromProject
		// This serializer only handles the index table

		progress = 1.0f;
	}

	bool AssetPackSerializer::DeserializeIndex(
		const std::filesystem::path& path,
		AssetPackFile& file
	)
	{
		FileStreamReader reader(path);
		if (!reader.IsStreamGood())
		{
			Log::CoreError("Failed to open asset pack file: {}", path.string());
			return false;
		}

		// Read and validate header
		char header[4];
		reader.ReadData(header, 4);
		if (std::memcmp(header, "IGPK", 4) != 0)
		{
			Log::CoreError("Invalid asset pack file: magic number mismatch");
			return false;
		}

		reader.ReadRaw(file.Header.Version);
		reader.ReadRaw(file.Header.BuildVersion);

		if (file.Header.Version != 1)
		{
			Log::CoreError("Unsupported asset pack version: {}", file.Header.Version);
			return false;
		}

		// Read index table
		DeserializeIndexTable(reader, file.Index);

		return true;
	}

	uint64_t AssetPackSerializer::CalculateIndexTableSize(const AssetPackFile& file)
	{
		uint64_t size = 0;

		// StartSceneHandle (UUID is 16 bytes)
		size += 16;

		// Script module offset and size
		size += sizeof(uint64_t) * 2;

		// Scenes map size
		size += sizeof(uint32_t);

		// For each scene
		for (const auto& [scene_handle, scene_info] : file.Index.Scenes)
		{
			// Scene handle (UUID)
			size += 16;
			// Scene offset, size, flags
			size += sizeof(uint64_t) * 2 + sizeof(uint16_t);
			// Assets map size
			size += sizeof(uint32_t);

			// For each asset in scene
			for (const auto& [asset_handle, asset_info] : scene_info.Assets)
			{
				// Asset handle (UUID)
				size += 16;
				// Asset offset, size, type, flags
				size += sizeof(uint64_t) * 2 + sizeof(uint16_t) * 2;
			}
		}

		return size;
	}

	void AssetPackSerializer::SerializeIndexTable(StreamWriter& writer, const AssetPackFile::IndexTable& index)
	{
		// Write start scene handle
		writer.WriteRaw(index.StartSceneHandle);

		// Write script module info
		writer.WriteRaw(index.PackedScriptModuleOffset);
		writer.WriteRaw(index.PackedScriptModuleSize);

		// Write scenes map
		writer.WriteRaw<uint32_t>((uint32_t)index.Scenes.size());
		for (const auto& [scene_handle, scene_info] : index.Scenes)
		{
			writer.WriteRaw(scene_handle);
			writer.WriteRaw(scene_info.PackedOffset);
			writer.WriteRaw(scene_info.PackedSize);
			writer.WriteRaw(scene_info.Flags);

			// Write assets map
			writer.WriteRaw<uint32_t>((uint32_t)scene_info.Assets.size());
			for (const auto& [asset_handle, asset_info] : scene_info.Assets)
			{
				writer.WriteRaw(asset_handle);
				writer.WriteRaw(asset_info.PackedOffset);
				writer.WriteRaw(asset_info.PackedSize);
				writer.WriteRaw(asset_info.Type);
				writer.WriteRaw(asset_info.Flags);
			}
		}
	}

	void AssetPackSerializer::DeserializeIndexTable(StreamReader& reader, AssetPackFile::IndexTable& index)
	{
		// Read start scene handle
		reader.ReadRaw(index.StartSceneHandle);

		// Read script module info
		reader.ReadRaw(index.PackedScriptModuleOffset);
		reader.ReadRaw(index.PackedScriptModuleSize);

		// Read scenes map
		uint32_t scene_count;
		reader.ReadRaw(scene_count);
		index.Scenes.clear();
		index.Scenes.reserve(scene_count);

		for (uint32_t i = 0; i < scene_count; ++i)
		{
			AssetHandle scene_handle;
			AssetPackFile::SceneInfo scene_info;

			reader.ReadRaw(scene_handle);
			reader.ReadRaw(scene_info.PackedOffset);
			reader.ReadRaw(scene_info.PackedSize);
			reader.ReadRaw(scene_info.Flags);

			// Read assets map
			uint32_t asset_count;
			reader.ReadRaw(asset_count);
			scene_info.Assets.reserve(asset_count);

			for (uint32_t j = 0; j < asset_count; ++j)
			{
				AssetHandle asset_handle;
				AssetPackFile::AssetInfo asset_info;

				reader.ReadRaw(asset_handle);
				reader.ReadRaw(asset_info.PackedOffset);
				reader.ReadRaw(asset_info.PackedSize);
				reader.ReadRaw(asset_info.Type);
				reader.ReadRaw(asset_info.Flags);

				scene_info.Assets[asset_handle] = asset_info;
			}

			index.Scenes[scene_handle] = scene_info;
		}
	}
}
