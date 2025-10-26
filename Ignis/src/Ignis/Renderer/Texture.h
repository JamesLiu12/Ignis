#pragma once

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

	struct TextureSpecs
	{
		uint32_t Width = 0;
		uint32_t Height = 0;
		ImageFormat SourceFormat = ImageFormat::RGBA;
		ImageFormat InternalFormat = ImageFormat::RGBA;
		TextureWrap WrapS = TextureWrap::Repeat;
		TextureWrap WrapT = TextureWrap::Repeat;
		TextureFilter MinFilter = TextureFilter::LinearMipmapLinear;
		TextureFilter MagFilter = TextureFilter::Linear;
		bool GenMipmaps = true;
	};

	class Texture
	{
	public:
		virtual ~Texture() = default;
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual void Bind(uint32_t unit) const = 0;
		virtual void UnBind() const = 0;
	};

	class Texture2D : public Texture
	{
	public:
		static std::shared_ptr<Texture2D> CreateFromFile(const TextureSpecs& specs, const std::string& filepath, bool flip_vertical = true);
		static std::shared_ptr<Texture2D> Create(const TextureSpecs& specs, std::span<const std::byte> data);
	};
}