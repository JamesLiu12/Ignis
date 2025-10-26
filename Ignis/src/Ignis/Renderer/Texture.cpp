#pragma once

#include "Texture.h"
#include "GraphicsAPI.h"
#include "Ignis/Platform/OpenGL/GLTexture.h"

namespace ignis
{
	std::shared_ptr<Texture2D> Texture2D::CreateFromFile(const TextureSpecs& specs, const std::string& filepath, bool flip_vertical)
	{
		switch (GraphicsAPI::GetType())
		{
		case GraphicsAPI::Type::OpenGL:
			return std::make_shared<GLTexture2D>(specs, filepath, flip_vertical);
		default:
			return nullptr;
		}
	}
	std::shared_ptr<Texture2D> Texture2D::Create(const TextureSpecs& specs, std::span<const std::byte> data)
	{
		switch (GraphicsAPI::GetType())
		{
		case GraphicsAPI::Type::OpenGL:
			return std::make_shared<GLTexture2D>(specs, data);
		default:
			return nullptr;
		}
	}
}