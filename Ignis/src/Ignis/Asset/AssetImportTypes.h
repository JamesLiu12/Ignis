#pragma once
#include "Ignis/Renderer/IBLBaker.h"

namespace ignis
{
	struct TextureImportOptions
	{
		bool FlipVertical = true;
		bool GenMipmaps = true;

		TextureFormat InternalFormat = TextureFormat::RGBA8;
		TextureWrap WrapS = TextureWrap::Repeat;
		TextureWrap WrapT = TextureWrap::Repeat;
		TextureFilter MinFilter = TextureFilter::LinearMipmapLinear;
		TextureFilter MagFilter = TextureFilter::Linear;
	};

	struct AssetLoadContext
	{
		std::shared_ptr<IBLBaker> IBLBakerService = nullptr;

		TextureImportOptions Texture2DOptions{};
		TextureImportOptions TextureCubeOptions{};
		TextureImportOptions EquirectOptions{};
		IBLBakeSettings EquirectBakeSettings{};
	};
}