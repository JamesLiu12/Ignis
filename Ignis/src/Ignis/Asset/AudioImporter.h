#pragma once
#include "AssetImporter.h"

namespace ignis
{
	class AudioImporter : public AssetImporter
	{
	public:
		AssetType              GetType()  const override;
		std::shared_ptr<Asset> Import(const std::string& path, const AssetLoadContext& context) override;

		static AudioImporter& Get();
	};
}