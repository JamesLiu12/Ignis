#pragma once

#include "Texture.h"
#include "GraphicsAPI.h"
#include "Ignis/Platform/OpenGL/GLTexture.h"

namespace ignis
{
	std::shared_ptr<Texture2D> Texture2D::Create(const TextureSpecs& specs, ImageFormat source_format, std::span<const std::byte> data)
	{
		switch (GraphicsAPI::GetType())
		{
		case GraphicsAPI::Type::OpenGL:
			return std::make_shared<GLTexture2D>(specs, source_format, data);
		default:
			return nullptr;
		}
	}

	std::shared_ptr<Texture2D> Texture2D::Create(const TextureSpecs& specs)
	{
		switch (GraphicsAPI::GetType())
		{
		case GraphicsAPI::Type::OpenGL:
			return std::make_shared<GLTexture2D>(specs);
		default:
			return nullptr;
		}
	}
	
	std::shared_ptr<Texture2D> Create(const glm::vec4 color)
	{
		const std::array<std::byte, 4> pixel = {
			std::byte{ static_cast<uint8_t>(color.r * 255.0f) },
			std::byte{ static_cast<uint8_t>(color.g * 255.0f) },
			std::byte{ static_cast<uint8_t>(color.b * 255.0f) },
			std::byte{ static_cast<uint8_t>(color.a * 255.0f) }
		};
		TextureSpecs specs;
		specs.Width = 1;
		specs.Height = 1;
		specs.Format = TextureFormat::RGBA8;
		return Texture2D::Create(specs, ImageFormat::RGBA8, std::span<const std::byte>(pixel));
	}

	std::shared_ptr<TextureCube> TextureCube::Create(const TextureSpecs& specs, ImageFormat source_format, std::span<const std::byte> data)
	{
		switch (GraphicsAPI::GetType())
		{
		case GraphicsAPI::Type::OpenGL:
			return std::make_shared<GLTextureCube>(specs, source_format, data);
		default:
			return nullptr;
		}
	}

	std::shared_ptr<TextureCube> TextureCube::Create(const TextureSpecs& specs)
	{
		switch (GraphicsAPI::GetType())
		{
		case GraphicsAPI::Type::OpenGL:
			return std::make_shared<GLTextureCube>(specs);
		default:
			return nullptr;
		}
	}
}