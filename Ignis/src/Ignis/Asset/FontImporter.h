#pragma once
#include "AssetImporter.h"

namespace ignis
{
	class FontImporter : public AssetImporter
	{
	public:
		AssetType              GetType()  const override;
		std::shared_ptr<Asset> Import(const AssetMetadata& metadata, const AssetLoadContext& context) override;

		static FontImporter& Get();
	};
}