#pragma once
#include <cstdint>

namespace ignis
{
	enum class TextureWrap
	{
		Repeat,
		MirroredRepeat,
		ClampToEdge,
		ClampToBorder
	};

	enum class TextureFilter
	{
		Nearest,
		Linear,
		NearestMipmapNearest,
		LinearMipmapNearest,
		NearestMipmapLinear,
		LinearMipmapLinear
	};

	enum class TextureFormat {
		None = 0,

		// Color LDR/HDR
		R8,
		RGBA8,
		RGBA8_sRGB,
		RGBA16F,
		RGBA32F,
		RG16F,
		RG32F,

		// Depth/stencil
		Depth24,
		Depth32F,
		Depth24Stencil8,
	};

	enum class TextureUsage : uint32_t
	{
		None = 0,
		Sampled = 1 << 0,
		RenderTarget = 1 << 1,
		Depth = 1 << 2,
		Stencil = 1 << 3,
	};

	inline TextureUsage operator|(TextureUsage a, TextureUsage b) {
		return (TextureUsage)((uint32_t)a | (uint32_t)b);
	}
	inline bool HasUsage(TextureUsage u, TextureUsage bit) {
		return ((uint32_t)u & (uint32_t)bit) != 0;
	}

	struct TextureSpecs
	{
		uint32_t Width = 0;
		uint32_t Height = 0;
		TextureFormat Format = TextureFormat::RGBA8;
		TextureWrap WrapS = TextureWrap::Repeat;
		TextureWrap WrapT = TextureWrap::Repeat;
		TextureFilter MinFilter = TextureFilter::LinearMipmapLinear;
		TextureFilter MagFilter = TextureFilter::Linear;
		bool GenMipmaps = true;

		uint32_t Samples = 1;
		TextureUsage Usage = TextureUsage::Sampled;
	};
}