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
		static std::shared_ptr<Texture2D> ImportTexture2D(const std::string& path, const TextureImportOptions& options = TextureImportOptions());
		static std::shared_ptr<TextureCube> ImportTextureCube(const std::string& path, const TextureImportOptions& options = TextureImportOptions());
	};
}