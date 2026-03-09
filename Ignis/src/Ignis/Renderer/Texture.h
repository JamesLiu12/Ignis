#pragma once

#include "Ignis/Asset/Asset.h"
#include "TextureTypes.h"
#include "Image.h"

#include <glm/glm.hpp>

namespace ignis
{
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
		static std::shared_ptr<Texture2D> Create(const glm::vec4 color);
	};

	class TextureCube : public Texture
	{
	public:
		static std::shared_ptr<TextureCube> Create(const TextureSpecs& specs, ImageFormat source_format, std::span<const std::byte> data);
		static std::shared_ptr<TextureCube> Create(const TextureSpecs& specs);
	};
}