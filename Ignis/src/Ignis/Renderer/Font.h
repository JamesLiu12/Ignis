#pragma once

#include "Ignis/Asset/Asset.h"
#include "Ignis/Renderer/Texture.h"

#include <glm/glm.hpp>
#include <unordered_map>

namespace ignis
{
	struct GlyphMetrics
	{
		glm::vec2 AtlasMin;
		glm::vec2 AtlasMax;
		glm::vec2 QuadMin;
		glm::vec2 QuadMax;
		float     Advance;
	};

	class Font : public Asset
	{
	public:
		static AssetType GetStaticType() { return AssetType::Font; }
		AssetType        GetAssetType() const override { return AssetType::Font; }

		const std::shared_ptr<Texture2D>& GetAtlas()     const { return m_atlas; }
		float                             GetLineHeight() const { return m_line_height; }

		const GlyphMetrics* GetGlyph(char c) const
		{
			auto it = m_glyphs.find(static_cast<uint32_t>(c));
			return it != m_glyphs.end() ? &it->second : nullptr;
		}

	private:
		std::shared_ptr<Texture2D>                 m_atlas;
		std::unordered_map<uint32_t, GlyphMetrics> m_glyphs;
		float                                      m_line_height = 0.0f;

		friend class FontImporter;
	};
}