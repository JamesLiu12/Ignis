#pragma once

#include "Ignis/Renderer/Texture.h"
#include "Ignis/Renderer/Image.h"

namespace ignis
{
	struct TextureImportOptions
	{
		bool FlipVertical = true;
		bool GenMipmaps = true;

		ImageFormat InternalFormat = ImageFormat::RGBA8;
		TextureWrap WrapS = TextureWrap::Repeat;
		TextureWrap WrapT = TextureWrap::Repeat;
		TextureFilter MinFilter = TextureFilter::LinearMipmapLinear;
		TextureFilter MagFilter = TextureFilter::Linear;
	};

	class TextureImporter
	{
	public:
		template<std::derived_from<Texture> T>
		static std::shared_ptr<T> ImportTexture(const std::string& path, const TextureImportOptions& options = TextureImportOptions())
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

			return T::Create(specs, image->GetPixels());
		}
	};
}