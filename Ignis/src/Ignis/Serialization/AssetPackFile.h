#pragma once

#include "Ignis/Asset/Asset.h"

namespace ignis
{
	struct AssetPackFile
	{
		struct FileHeader
		{
			char HEADER[4] = {'I','G','P','K'};
			uint32_t Version = 1;
			uint64_t BuildVersion = 0;
		};

		struct AssetInfo
		{
			uint64_t PackedOffset = 0;
			uint64_t PackedSize = 0;
			uint16_t Type = 0;
			uint16_t Flags = 0;
		};

		struct SceneInfo
		{
			uint64_t PackedOffset = 0;
			uint64_t PackedSize = 0;
			uint16_t Flags = 0;
			std::unordered_map<AssetHandle, AssetInfo> Assets;
		};

		struct IndexTable
		{
			AssetHandle StartSceneHandle = AssetHandle::Invalid;
			uint64_t PackedScriptModuleOffset = 0;
			uint64_t PackedScriptModuleSize = 0;
			std::unordered_map<AssetHandle, SceneInfo> Scenes;
		};

		FileHeader Header;
		IndexTable Index;
	};

	struct AssetSerializationInfo
	{
		uint64_t Offset = 0;
		uint64_t Size = 0;
	};
}
