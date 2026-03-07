#include "FontImporter.h"
#include "Ignis/Renderer/Font.h"
#include "AssetLoadContext.h"

#include <stb_truetype.h>
#include <fstream>
#include <filesystem>

namespace ignis
{
	AssetType FontImporter::GetType() const { return AssetType::Font; }

	std::shared_ptr<Asset> FontImporter::Import(const AssetMetadata& metadata, const AssetLoadContext& context)
	{
		const auto* opts = std::get_if<FontImportOptions>(&metadata.ImportOptions);
		const FontImportOptions& options = opts ? *opts : FontImportOptions{};

		std::filesystem::path resolved = VFS::Resolve(metadata.FilePath);
		std::ifstream file(resolved, std::ios::binary | std::ios::ate);
		if (!file)
		{
			Log::CoreError("FontImporter: Cannot open '{}'", metadata.FilePath);
			return nullptr;
		}
		std::vector<uint8_t> ttf((size_t)file.tellg());
		file.seekg(0);
		file.read(reinterpret_cast<char*>(ttf.data()), (std::streamsize)ttf.size());

		constexpr int kFirst = 32, kCount = 95;
		std::vector<stbtt_packedchar> packed(kCount);
		std::vector<uint8_t>          bitmap(options.AtlasWidth * options.AtlasHeight, 0);

		stbtt_pack_context ctx;
		if (!stbtt_PackBegin(&ctx, bitmap.data(), (int)options.AtlasWidth, (int)options.AtlasHeight, 0, 1, nullptr))
		{
			Log::CoreError("FontImporter: stbtt_PackBegin failed for '{}'", metadata.FilePath);
			return nullptr;
		}
		stbtt_PackSetOversampling(&ctx, 2, 2);
		stbtt_PackFontRange(&ctx, ttf.data(), 0, options.FontSize, kFirst, kCount, packed.data());
		stbtt_PackEnd(&ctx);

		stbtt_fontinfo info;
		stbtt_InitFont(&info, ttf.data(), 0);
		float scale = stbtt_ScaleForPixelHeight(&info, options.FontSize);
		int asc, desc, gap;
		stbtt_GetFontVMetrics(&info, &asc, &desc, &gap);
		float line_height = (asc - desc + gap) * scale;

		float inv_w = 1.0f / (float)options.AtlasWidth;
		float inv_h = 1.0f / (float)options.AtlasHeight;

		auto font = std::make_shared<Font>();
		font->m_line_height = line_height;

		for (int i = 0; i < kCount; ++i)
		{
			const stbtt_packedchar& p = packed[i];
			GlyphMetrics g;
			g.AtlasMin = { p.x0 * inv_w, p.y0 * inv_h };
			g.AtlasMax = { p.x1 * inv_w, p.y1 * inv_h };
			g.QuadMin = { p.xoff,  p.yoff };
			g.QuadMax = { p.xoff2, p.yoff2 };
			g.Advance = p.xadvance;
			font->m_glyphs[static_cast<uint32_t>(kFirst + i)] = g;
		}

		TextureSpecs specs;
		specs.Width = options.AtlasWidth;
		specs.Height = options.AtlasHeight;
		specs.Format = TextureFormat::R8;
		specs.WrapS = TextureWrap::ClampToEdge;
		specs.WrapT = TextureWrap::ClampToEdge;
		specs.MinFilter = TextureFilter::Linear;
		specs.MagFilter = TextureFilter::Linear;
		specs.GenMipmaps = false;

		std::vector<std::byte> r8(bitmap.size());
		for (size_t i = 0; i < bitmap.size(); ++i)
			r8[i] = (std::byte)bitmap[i];

		font->m_atlas = Texture2D::Create(specs, ImageFormat::R8, r8);
		if (!font->m_atlas)
		{
			Log::CoreError("FontImporter: GPU upload failed for '{}'", metadata.FilePath);
			return nullptr;
		}

		Log::CoreInfo("FontImporter: Loaded '{}' ({:.0f}px, {}x{} atlas)",
			metadata.FilePath, options.FontSize, options.AtlasWidth, options.AtlasHeight);
		return font;
	}

	FontImporter& FontImporter::Get()
	{
		static FontImporter instance;
		return instance;
	}
}