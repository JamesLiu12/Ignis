#pragma once

#include "Ignis/Asset/Asset.h"
#include "Image.h"

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
		None         = 0,
		Sampled		 = 1 << 0,
		RenderTarget = 1 << 1,
		Depth		 = 1 << 2,
		Stencil		 = 1 << 3,
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

	class Texture : public Asset
	{
	public:
		virtual ~Texture() = default;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		virtual void SetData(ImageFormat source_format, std::span<const std::byte> data) const = 0;

		virtual void Bind(uint32_t unit) const = 0;
		virtual void UnBind() const = 0;
	};

	class Texture2D : public Texture
	{
	public:
		static std::shared_ptr<Texture2D> Create(const TextureSpecs& specs, ImageFormat source_format, std::span<const std::byte> data);
		static std::shared_ptr<Texture2D> Create(const TextureSpecs& specs);
	};

	class TextureCube : public Texture
	{
	public:
		static std::shared_ptr<TextureCube> Create(const TextureSpecs& specs, ImageFormat source_format, std::span<const std::byte> data);
		static std::shared_ptr<TextureCube> Create(const TextureSpecs& specs);
	};
}