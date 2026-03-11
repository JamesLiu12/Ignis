#pragma once

#include "AssetPackFile.h"
#include "FileStream.h"

namespace ignis
{
	class IGNIS_API AssetPackSerializer
	{
	public:
		static void Serialize(
			const std::filesystem::path& path,
			AssetPackFile& file,
			const std::vector<uint8_t>& script_module,
			std::atomic<float>& progress
		);

		static bool DeserializeIndex(
			const std::filesystem::path& path,
			AssetPackFile& file
		);

		static void SerializeIndexTable(StreamWriter& writer, const AssetPackFile::IndexTable& index);
		static void DeserializeIndexTable(StreamReader& reader, AssetPackFile::IndexTable& index);

	private:
		static uint64_t CalculateIndexTableSize(const AssetPackFile& file);
	};
}
