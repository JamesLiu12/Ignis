#include "Renderer.h"
#include "GraphicsAPI.h"
#include "Ignis/Platform/OpenGL/GLRenderer.h"
#include "Texture.h"

namespace ignis
{
	std::unique_ptr<Renderer> Renderer::Create()
	{
		switch (GraphicsAPI::GetType())
		{
		case GraphicsAPI::Type::OpenGL:
			return std::make_unique<GLRenderer>();
		default:
			return nullptr;
		}
	}

	const std::shared_ptr<Texture2D> Renderer::GetWhiteTexture()
	{
		static constexpr std::array<std::byte, 4> s_white_pixel =
		{ std::byte{ 255 } ,std::byte{ 255 } ,std::byte{ 255 } ,std::byte{ 255 } };
		static const std::span<const std::byte> s_white_data(s_white_pixel);

		ignis::TextureSpecs specs;
		specs.Width = 1;
		specs.Height = 1;
		specs.SourceFormat = ignis::ImageFormat::RGBA8;
		specs.InternalFormat = ignis::ImageFormat::RGBA8;

		return ignis::Texture2D::Create(specs, s_white_data);
	}

	const std::shared_ptr<Texture2D> Renderer::GetBlackTexture()
	{
		static constexpr std::array<std::byte, 4> s_black_pixel =
		{ std::byte{ 0 } ,std::byte{ 0 } ,std::byte{ 0 } ,std::byte{ 0 } };
		static const std::span<const std::byte> s_black_data(s_black_pixel);

		ignis::TextureSpecs specs;
		specs.Width = 1;
		specs.Height = 1;
		specs.SourceFormat = ignis::ImageFormat::RGBA8;
		specs.InternalFormat = ignis::ImageFormat::RGBA8;

		return ignis::Texture2D::Create(specs, s_black_data);
	}
}