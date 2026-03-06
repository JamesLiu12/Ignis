#include "UIRenderer.h"

#include "Ignis/Renderer/Renderer.h"
#include "Ignis/Renderer/Material.h"
#include "Ignis/Renderer/Font.h"
#include "Ignis/Asset/AssetManager.h"

#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

namespace ignis
{
	UIRenderer::UIRenderer(Renderer& renderer)
		: m_renderer(renderer)
	{
	}

	void UIRenderer::BeginUI(uint32_t screen_width, uint32_t screen_height)
	{
		m_screen_width = screen_width;
		m_screen_height = screen_height;

		// Y-down: (0,0) = top-left, used for rects
		m_projection = glm::ortho(
			0.0f, static_cast<float>(screen_width),
			static_cast<float>(screen_height), 0.0f,
			-1.0f, 1.0f);

		// Y-up: used for text, keeps the same vertex-building convention
		m_text_projection = glm::ortho(
			0.0f, static_cast<float>(screen_width),
			0.0f, static_cast<float>(screen_height),
			-1.0f, 1.0f);

		m_rect_items.clear();
		m_text_items.clear();
	}

	void UIRenderer::EndUI()
	{
		auto sort_fn = [](const auto& a, const auto& b)
			{
				if (a.SortOrder != b.SortOrder) return a.SortOrder < b.SortOrder;
				return a.Depth < b.Depth;
			};

		std::sort(m_rect_items.begin(), m_rect_items.end(), sort_fn);
		std::sort(m_text_items.begin(), m_text_items.end(), sort_fn);

		FlushRects();
		FlushTexts();
	}

	void UIRenderer::SubmitRect(const glm::vec2& min, const glm::vec2& max,
		const glm::vec4& color,
		std::shared_ptr<Texture2D> texture,
		int sort_order, float depth)
	{
		m_rect_items.push_back({ min, max, color, std::move(texture), sort_order, depth });
	}

	void UIRenderer::SubmitText(const glm::vec2& rect_min, const glm::vec2& rect_size,
		const std::string& text, std::shared_ptr<Font> font,
		const glm::vec4& color, float font_size,
		UITextComponent::HorizontalAlignment h_align,
		UITextComponent::VerticalAlignment   v_align,
		int sort_order, float depth)
	{
		m_text_items.push_back({
			rect_min, rect_size, text, std::move(font),
			color, font_size, h_align, v_align,
			sort_order, depth
			});
	}

	void UIRenderer::FlushRects()
	{
		auto ui_shader = m_renderer.GetShaderLibrary().Get("UI");
		if (!ui_shader) return;

		for (const auto& item : m_rect_items)
		{
			auto mat = Material::Create(ui_shader);
			mat->Set("u_Projection", m_projection);
			mat->Set("u_Color", item.Color);

			if (item.Texture)
			{
				mat->Set("u_UseTexture", 1);
				mat->Set("u_Texture", item.Texture);
			}
			else
			{
				mat->Set("u_UseTexture", 0);
				mat->Set("u_Texture", Renderer::GetWhiteTexture());
			}

			mat->Bind();
			m_renderer.RenderSprite(item.Min, item.Max);
		}
	}

	void UIRenderer::FlushTexts()
	{
		for (const auto& item : m_text_items)
		{
			if (!item.FontPtr || item.Text.empty()) continue;

			float scale = item.FontPtr->GetLineHeight() > 0.0f
				? item.FontSize / item.FontPtr->GetLineHeight()
				: 1.0f;

			// Compute text width for alignment (single-line)
			float text_w = 0.0f;
			for (char c : item.Text)
			{
				if (c == '\n') break;
				if (const GlyphMetrics* g = item.FontPtr->GetGlyph(c))
					text_w += g->Advance * scale;
			}
			float text_h = item.FontPtr->GetLineHeight() * scale;

			// Alignment in the bounding rect (y-down)
			float offset_x = 0.0f;
			switch (item.HAlign)
			{
			case UITextComponent::HorizontalAlignment::Center:
				offset_x = (item.RectSize.x - text_w) * 0.5f; break;
			case UITextComponent::HorizontalAlignment::Right:
				offset_x = item.RectSize.x - text_w; break;
			default: break;
			}

			float offset_y = 0.0f;
			switch (item.VAlign)
			{
			case UITextComponent::VerticalAlignment::Middle:
				offset_y = (item.RectSize.y - text_h) * 0.5f; break;
			case UITextComponent::VerticalAlignment::Bottom:
				offset_y = item.RectSize.y - text_h; break;
			default: break;
			}

			// Screen position (y-down) -> convert to y-up for text_projection
			float screen_x = item.RectMin.x + offset_x;
			float screen_y = (static_cast<float>(m_screen_height)
				- (item.RectMin.y + offset_y));  // flip y

			glm::mat4 model = glm::translate(glm::mat4(1.0f),
				{ screen_x, screen_y, 0.0f });

			m_renderer.RenderUIText(
				*item.FontPtr, item.Text,
				m_text_projection, model,
				item.Color, scale);
		}
	}
}