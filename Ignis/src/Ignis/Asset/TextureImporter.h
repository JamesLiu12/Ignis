#pragma once

#include "Ignis/Renderer/Texture.h"
#include "Ignis/Renderer/Image.h"
#include "Ignis/Renderer/Environment.h"
#include "AssetImporter.h"

namespace ignis
{
	class Texture2DImporter : public AssetImporter
	{
	public:
		AssetType GetType() const override;
		std::shared_ptr<Asset> Import(const std::string& path, const AssetLoadContext& context) override;

		static Texture2DImporter& Get();
	};

	class TextureCubeImporter : public AssetImporter
	{
	public:
		AssetType GetType() const override;
		std::shared_ptr<Asset> Import(const std::string& path, const AssetLoadContext& context) override;

		static TextureCubeImporter& Get();
	};

	class EquirectEnvImporter : public AssetImporter
	{
	public:
		AssetType GetType() const override;
		std::shared_ptr<Asset> Import(const std::string& path, const AssetLoadContext& context) override;

		static EquirectEnvImporter& Get();
	};
}