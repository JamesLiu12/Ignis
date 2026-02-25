#include "AssetSerializer.h"
#include "Ignis/Core/File/File.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;

namespace ignis
{
	static ordered_json SerializeMetadata(const AssetMetadata& meta)
	{
		ordered_json data;
		data["Handle"] = meta.Handle.ToString();
		data["Type"] = static_cast<int>(meta.Type);
		data["FilePath"] = meta.FilePath.string();
		return data;
	}

	static bool DeserializeMetadata(const ordered_json& data, AssetMetadata& out_meta)
	{
		if (!data.contains("Handle") || !data["Handle"].is_string())
			return false;
		if (!data.contains("Type") || !data["Type"].is_number_integer())
			return false;
		if (!data.contains("FilePath") || !data["FilePath"].is_string())
			return false;

		out_meta.Handle = UUID(data["Handle"].get<std::string>());
		out_meta.Type = static_cast<AssetType>(data["Type"].get<int>());
		out_meta.FilePath = data["FilePath"].get<std::string>();

		return true;
	}

	bool AssetSerializer::Serialize(const std::unordered_map<AssetHandle, AssetMetadata>& asset_registry, const std::filesystem::path& filepath)
	{
		File file(filepath);
		auto stream = file.OpenOutputStream();
		if (!stream.is_open())
		{
			return false;
			Log::CoreError("[AssetSerializer::Serialize] Failed to open file for writing");
		}

		ordered_json data;
		data["Assets"] = ordered_json::array();

		std::vector<std::pair<AssetHandle, AssetMetadata>> sorted_assets(
			asset_registry.begin(),
			asset_registry.end()
		);

		std::sort(sorted_assets.begin(), sorted_assets.end(),
			[](const auto& a, const auto& b) {
				return a.first < b.first;
			});

		for (const auto& [asset_handle, meta_data] : sorted_assets)
		{
			AssetMetadata meta = meta_data;
			meta.Handle = asset_handle;

			data["Assets"].push_back(SerializeMetadata(meta));
		}

		try
		{
			stream << data.dump(4);
			stream.close();
			Log::CoreInfo("[AssetSerializer::Serialize] Successfully serialized asset to: {}", file.GetPath().string());
			return true;
		}
		catch (const std::exception& e)
		{
			Log::CoreError("[AssetSerializer::Serialize] Failed to write JSON: {}", e.what());
			return false;
		}
	}

	std::optional<std::unordered_map<AssetHandle, AssetMetadata>> AssetSerializer::Deserialize(const std::filesystem::path& filepath)
	{
		std::unordered_map<AssetHandle, AssetMetadata> registry;

		File file(filepath);
		auto stream = file.OpenInputStream();
		if (!stream.is_open())
		{
			Log::CoreError("[AssetSerializer::Deserialize] Failed to open file for reading: {}",
				filepath.string());
			return {};
		}

		ordered_json data;
		try
		{
			stream >> data;
		}
		catch (const std::exception& e)
		{
			Log::CoreError("[AssetSerializer::Deserialize] Failed to parse JSON: {}", e.what());
			return {};
		}

		if (!data.contains("Assets") || !data["Assets"].is_array())
		{
			Log::CoreError("[AssetSerializer::Deserialize] Invalid format: missing 'Assets' array");
			return {};
		}

		for (const auto& item : data["Assets"])
		{
			AssetMetadata meta;
			if (!DeserializeMetadata(item, meta))
			{
				Log::CoreError("[AssetSerializer::Deserialize] Skipping invalid asset entry");
				continue;
			}

			const AssetHandle handle = meta.Handle;
			registry[handle] = std::move(meta);
		}

		Log::CoreInfo("[AssetSerializer::Deserialize] Successfully deserialized assets from: {}",
			file.GetPath().string());
		return registry;
	}
}