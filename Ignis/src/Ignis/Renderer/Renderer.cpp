#include "Renderer.h"
#include "GraphicsAPI.h"
#include "Ignis/Platform/OpenGL/GLRenderer.h"
#include "Texture.h"
#include "Ignis/Asset/AssetManager.h"

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

	std::shared_ptr<Texture2D> Renderer::GetWhiteTexture()
	{
		static constexpr std::array<std::byte, 4> s_white_pixel =
		{ std::byte{ 255 } ,std::byte{ 255 } ,std::byte{ 255 } ,std::byte{ 255 } };
		static const std::span<const std::byte> s_white_data(s_white_pixel);

		static std::shared_ptr<Texture2D> s_texture;

		if (!s_texture)
		{
			ignis::TextureSpecs specs;
			specs.Width = 1;
			specs.Height = 1;
			specs.Format = TextureFormat::RGBA8;

			s_texture = ignis::Texture2D::Create(specs, ImageFormat::RGBA8, s_white_data);
		}

		return s_texture;
	}

	std::shared_ptr<Texture2D> Renderer::GetBlackTexture()
	{
		static constexpr std::array<std::byte, 4> s_black_pixel =
		{ std::byte{ 0 } ,std::byte{ 0 } ,std::byte{ 0 } ,std::byte{ 0 } };
		static const std::span<const std::byte> s_black_data(s_black_pixel);

		static std::shared_ptr<Texture2D> s_texture;

		if (!s_texture)
		{
			ignis::TextureSpecs specs;
			specs.Width = 1;
			specs.Height = 1;
			specs.Format = ignis::TextureFormat::RGBA8;

			s_texture = ignis::Texture2D::Create(specs, ImageFormat::RGBA8, s_black_data);
		}

		return s_texture;
	}

	std::shared_ptr<Texture2D> Renderer::GetDefaultNormalTexture()
	{
		static constexpr std::array<std::byte, 4> s_normal_pixel =
		{ std::byte{ 128 }, std::byte{ 128 }, std::byte{ 255 }, std::byte{ 255 } };
		static const std::span<const std::byte> s_normal_data(s_normal_pixel);

		static std::shared_ptr<Texture2D> s_texture;

		if (!s_texture)
		{
			ignis::TextureSpecs specs;
			specs.Width = 1;
			specs.Height = 1;
			specs.Format = ignis::TextureFormat::RGBA8;

			s_texture = ignis::Texture2D::Create(specs, ImageFormat::RGBA8, s_normal_data);
		}

		return s_texture;
	}

	std::shared_ptr<Texture2D> Renderer::GetDefaultRoughnessTexture()
	{
		static constexpr std::array<std::byte, 4> s_rough_pixel =
		{ std::byte{ 128 }, std::byte{ 128 }, std::byte{ 128 }, std::byte{ 255 } };
		static const std::span<const std::byte> s_rough_data(s_rough_pixel);

		static std::shared_ptr<Texture2D> s_texture;

		if (!s_texture)
		{
			ignis::TextureSpecs specs;
			specs.Width = 1;
			specs.Height = 1;
			specs.Format = ignis::TextureFormat::RGBA8;

			s_texture = ignis::Texture2D::Create(specs, ImageFormat::RGBA8, s_rough_data);
		}

		return s_texture;
	}

	AssetHandle Renderer::GetWhiteTextureHandle()
	{
		static AssetHandle s_handle = AssetHandle::Invalid;
		
		if (!s_handle.IsValid())
		{
			auto texture = GetWhiteTexture();
			s_handle = AssetManager::AddMemoryOnlyAsset(texture);
		}
		
		return s_handle;
	}

	AssetHandle Renderer::GetBlackTextureHandle()
	{
		static AssetHandle s_handle = AssetHandle::Invalid;
		
		if (!s_handle.IsValid())
		{
			auto texture = GetBlackTexture();
			s_handle = AssetManager::AddMemoryOnlyAsset(texture);
		}
		
		return s_handle;
	}

	AssetHandle Renderer::GetDefaultNormalTextureHandle()
	{
		static AssetHandle s_handle = AssetHandle::Invalid;
		
		if (!s_handle.IsValid())
		{
			auto texture = GetDefaultNormalTexture();
			s_handle = AssetManager::AddMemoryOnlyAsset(texture);
		}
		
		return s_handle;
	}

	AssetHandle Renderer::GetDefaultRoughnessTextureHandle()
	{
		static AssetHandle s_handle = AssetHandle::Invalid;
		
		if (!s_handle.IsValid())
		{
			auto texture = GetDefaultRoughnessTexture();
			s_handle = AssetManager::AddMemoryOnlyAsset(texture);
		}
		
		return s_handle;
	}
	
}  // namespace ignis