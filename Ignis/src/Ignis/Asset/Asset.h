#pragma once

#include "Ignis/Core/UUID.h"

namespace ignis
{
	using AssetHandle = UUID;

	enum class AssetType
	{
		Unknown = 0,
		Texture,
		Mesh,
		EnvironmentMap
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
		AssetHandle m_handle = AssetHandle::InvalidUUID;
	};
}