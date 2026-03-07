#pragma once

#include "AssetImportOptions.h"
#include "Ignis/Core/UUID.h"
#include "AssetType.h"

#include <filesystem>

namespace ignis
{
	using AssetHandle = UUID;

	struct AssetMetadata
	{
		AssetHandle Handle = AssetHandle::Invalid;
		AssetType Type;
		std::string FilePath;
		AssetImportOptions ImportOptions;
	};

	class Asset
	{
	public:
		Asset() = default;
		virtual ~Asset() = default;
		
		AssetHandle GetHandle() const { return m_handle; }
		virtual AssetType GetAssetType() const { return AssetType::Unknown; }

		virtual bool operator==(const Asset& other) const { return m_handle == other.m_handle; }
		virtual bool operator!=(const Asset& other) const { return m_handle != other.m_handle; }

		friend class AssetManager;
	private:
		AssetHandle m_handle = AssetHandle::Invalid;
	};
}