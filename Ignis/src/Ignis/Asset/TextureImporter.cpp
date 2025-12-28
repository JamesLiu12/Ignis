#include "TextureImporter.h"

namespace ignis
{
	std::shared_ptr<Texture2D> TextureImporter::ImportTexture2D(const std::string& path, const TextureImportOptions& options)
	{
		std::filesystem::path resolved = VFS::Resolve(path);
		auto image = Image::LoadFromFile(resolved, options.FlipVertical);

		if (!image)
		{
			Log::CoreError("Failed to load texture from file: {}", path);
			return nullptr;
		}

		TextureSpecs specs;

		specs.Width = image->GetWidth();
		specs.Height = image->GetHeight();
		specs.SourceFormat = image->GetFormat();

		specs.InternalFormat = options.InternalFormat;
		specs.WrapS = options.WrapS;
		specs.WrapT = options.WrapT;
		specs.MinFilter = options.MinFilter;
		specs.MagFilter = options.MagFilter;
		specs.GenMipmaps = options.GenMipmaps;

		return Texture2D::Create(specs, image->GetPixels());
	}
}