#include "TextureImporter.h"
#include "Ignis/Renderer/IBLBaker.h"

namespace ignis
{
	AssetType Texture2DImporter::GetType() const
	{
		return AssetType::Texture2D;
	}

	std::shared_ptr<Asset> Texture2DImporter::Import(const std::string& path, const AssetLoadContext& context)
	{
		const TextureImportOptions& options = context.Texture2DOptions;

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

		specs.Format = options.InternalFormat;
		specs.WrapS = options.WrapS;
		specs.WrapT = options.WrapT;
		specs.MinFilter = options.MinFilter;
		specs.MagFilter = options.MagFilter;
		specs.GenMipmaps = options.GenMipmaps;

		return Texture2D::Create(specs, image->GetFormat(), image->GetPixels());
	}

	Texture2DImporter& Texture2DImporter::Get()
	{
		static Texture2DImporter instance;
		return instance;
	}

	AssetType TextureCubeImporter::GetType() const
	{
		return AssetType::TextureCube;
	}

	std::shared_ptr<Asset> TextureCubeImporter::Import(const std::string& path, const AssetLoadContext& context)
	{
		const TextureImportOptions& options = context.TextureCubeOptions;

		std::filesystem::path resolved = VFS::Resolve(path);

		auto image = Image::LoadFromFile(resolved, options.FlipVertical);

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
		specs.Format = options.InternalFormat;
		specs.WrapS = options.WrapS;
		specs.WrapT = options.WrapT;
		specs.MinFilter = options.MinFilter;
		specs.MagFilter = options.MagFilter;
		specs.GenMipmaps = options.GenMipmaps;

		uint32_t bpp = BytesPerPixel(image->GetFormat());
		uint32_t face_size = specs.Width * specs.Height * bpp;

		if (is_vertical)
		{
			return TextureCube::Create(specs, image->GetFormat(), image->GetPixels());
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

		return TextureCube::Create(specs, image->GetFormat(), reordered_data);
	}

	TextureCubeImporter& TextureCubeImporter::Get()
	{
		static TextureCubeImporter instance;
		return instance;
	}

	AssetType EquirectEnvImporter::GetType() const
	{
		return AssetType::EquirectIBLEnv;
	}

	std::shared_ptr<Asset> EquirectEnvImporter::Import(const std::string& path, const AssetLoadContext& context)
	{
		const TextureImportOptions& options = context.EquirectOptions;
		auto ibl_baker = context.IBLBakerService;

		std::filesystem::path resolved = VFS::Resolve(path);

		auto image = Image::LoadFromFile(resolved, options.FlipVertical);

		if (!image)
		{
			Log::CoreError("Failed to load texture from file: {}", path);
			return nullptr;
		}

		auto bake_result = ibl_baker->BakeFromEquirectangular(*image);

		auto environment = std::make_shared<Environment>();
		environment->SetSkyboxMap(bake_result.EnvironmentCube);
		environment->SetIBLMaps({
			bake_result.IrradianceCube,
			bake_result.PrefilterCube,
			bake_result.BrdfLUT
			});

		return environment;
	}

	EquirectEnvImporter& EquirectEnvImporter::Get()
	{
		static EquirectEnvImporter instance;
		return instance;
	}
}