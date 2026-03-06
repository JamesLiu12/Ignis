#pragma once

#include "Ignis/Core/API.h"
#include "UIComponents.h"

#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <string>

namespace ignis
{
	class Renderer;
	class Texture2D;
	class Font;

	struct UIRectItem
	{
		glm::vec2                    Min;
		glm::vec2                    Max;
		glm::vec4                    Color;
		std::shared_ptr<Texture2D>   Texture;  // null = solid color
		int                          SortOrder = 0;
		float                        Depth = 0.0f;
	};

	struct UITextItem
	{
		glm::vec2                         RectMin;
		glm::vec2                         RectSize;
		std::string                       Text;
		std::shared_ptr<Font>             FontPtr;
		glm::vec4                         Color;
		float                             FontSize;
		UITextComponent::HorizontalAlignment HAlign;
		UITextComponent::VerticalAlignment   VAlign;
		int                               SortOrder = 0;
		float                             Depth = 0.0f;
	};

	class IGNIS_API UIRenderer
	{
	public:
		explicit UIRenderer(Renderer& renderer);
		~UIRenderer() = default;

		// Call before submitting any UI items
		void BeginUI(uint32_t screen_width, uint32_t screen_height);

		// Sorts and flushes all queued items
		void EndUI();

		void SubmitRect(const glm::vec2& min, const glm::vec2& max,
			const glm::vec4& color,
			std::shared_ptr<Texture2D> texture = nullptr,
			int sort_order = 0, float depth = 0.0f);

		void SubmitText(const glm::vec2& rect_min, const glm::vec2& rect_size,
			const std::string& text, std::shared_ptr<Font> font,
			const glm::vec4& color, float font_size,
			UITextComponent::HorizontalAlignment h_align,
			UITextComponent::VerticalAlignment   v_align,
			int sort_order = 0, float depth = 0.0f);

	private:
		void FlushRects();
		void FlushTexts();

		Renderer& m_renderer;
		uint32_t               m_screen_width = 0;
		uint32_t               m_screen_height = 0;
		glm::mat4              m_projection;   // ortho, Y-down
		glm::mat4              m_text_projection; // ortho, Y-up (for text vertex convention)

		std::vector<UIRectItem> m_rect_items;
		std::vector<UITextItem> m_text_items;
	};
}