#pragma once

#include "Ignis/Renderer/IBLBaker.h"
#include "AssetImportTypes.h"

namespace ignis
{
	class AssetImporter
	{
	public:
		virtual ~AssetImporter() = default;
		virtual AssetType GetType() const = 0;
		virtual std::shared_ptr<Asset> Import(const std::string& path, const AssetLoadContext& context) = 0;
	};
}