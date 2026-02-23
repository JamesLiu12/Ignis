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