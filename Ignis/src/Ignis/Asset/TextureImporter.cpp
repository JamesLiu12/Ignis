#include "TextureImporter.h"

namespace ignis
{
	std::shared_ptr<Texture2D> TextureImporter::ImportTexture2D(const std::string& path, const TextureImportOptions& options)
	{
		std::filesystem::path resolved = VFS::Resolve(path);
		auto image = Image::LoadFromFile(resolved, options.FlipVertical);

		if (!image)
		{
			Log::CoreError("Failed to load texture from file: {}", path);
			return nullptr;
		}

		TextureSpecs specs;

		specs.Width = image->GetWidth();
		specs.Height = image->GetHeight();
		specs.SourceFormat = image->GetFormat();

		specs.InternalFormat = options.InternalFormat;
		specs.WrapS = options.WrapS;
		specs.WrapT = options.WrapT;
		specs.MinFilter = options.MinFilter;
		specs.MagFilter = options.MagFilter;
		specs.GenMipmaps = options.GenMipmaps;

		return Texture2D::Create(specs, image->GetPixels());
	}
	std::shared_ptr<TextureCube> TextureImporter::ImportTextureCube(const std::string& path, const TextureImportOptions& options)
	{
		std::filesystem::path resolved = VFS::Resolve(path);

		bool flip = false;
		auto image = Image::LoadFromFile(resolved, flip);

		if (!image)
		{
			Log::CoreError("Failed to load texture from file: {}", path);
			return nullptr;
		}

		TextureSpecs specs;
		uint32_t width = image->GetWidth();
		uint32_t height = image->GetHeight();

		bool is_horizontal = false;
		bool is_vertical = false;

		if (width / 6 == height)
		{
			is_horizontal = true;
			width /= 6;
		}
		else if (height / 6 == width)
		{
			is_vertical = true;
			height /= 6;
		}
		else
		{
			Log::CoreError("TextureCube import error: Image aspect ratio must be 6:1 or 1:6. Got {}x{}", width, height);
			return nullptr;
		}

		specs.Width = width;
		specs.Height = height;
		specs.SourceFormat = image->GetFormat();
		specs.InternalFormat = options.InternalFormat;
		specs.WrapS = options.WrapS;
		specs.WrapT = options.WrapT;
		specs.MinFilter = options.MinFilter;
		specs.MagFilter = options.MagFilter;
		specs.GenMipmaps = options.GenMipmaps;

		uint32_t bpp = BytesPerPixel(specs.SourceFormat);
		uint32_t face_size = specs.Width * specs.Height * bpp;

		if (is_vertical)
		{
			return TextureCube::Create(specs, image->GetPixels());
		}

		std::vector<std::byte> reordered_data(face_size * 6);

		const std::byte* srcData = image->GetPixels().data();
		std::byte* dstData = reordered_data.data();

		uint32_t total_stride = image->GetWidth() * bpp;
		uint32_t face_row_stride = specs.Width * bpp;

		for (int i = 0; i < 6; ++i)
		{
			for (int y = 0; y < specs.Height; ++y)
			{
				size_t src_offset = (y * total_stride) + (i * face_row_stride);

				size_t dst_offset = (i * face_size) + (y * face_row_stride);

				std::memcpy(dstData + dst_offset, srcData + src_offset, face_row_stride);
			}
		}

		return TextureCube::Create(specs, reordered_data);
	}
}