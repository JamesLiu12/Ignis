#pragma once
#include "Ignis/Renderer/Texture.h"
#include "Ignis/Renderer/Image.h"

namespace ignis
{
	struct IBLBakeSettings
	{
		uint32_t EnvironmentResolution = 1024;
		uint32_t IrradianceResolution = 32;
		uint32_t PrefilterResolution = 256;
		uint32_t BrdfLUTResolution = 512;
	};

	struct IBLBakeResult
	{
		std::shared_ptr<TextureCube> EnvironmentCube;
		std::shared_ptr<TextureCube> IrradianceCube;
		std::shared_ptr<TextureCube> PrefilterCube;
		std::shared_ptr<Texture2D>   BrdfLUT;
		uint32_t PrefilterMipLevels = 1;
	};

	class Renderer;

	class IBLBaker
	{
	public:
		virtual IBLBakeResult BakeFromEquirectangular(
			const Image& hdr_image,
			const IBLBakeSettings& settings = {}
		) const = 0;

		static std::unique_ptr<IBLBaker> Create(Renderer& renderer);
	};
}