#pragma once

#include "Asset.h"
#include "AssetLoadContext.h"

namespace ignis
{
	class AssetImporter
	{
	public:
		virtual ~AssetImporter() = default;
		virtual AssetType GetType() const = 0;
		virtual std::shared_ptr<Asset> Import(const AssetMetadata& metadata, const AssetLoadContext& context) = 0;
	};
}