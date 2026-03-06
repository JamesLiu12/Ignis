#include "UISystem.h"

#include "Ignis/Scene/Scene.h"
#include "Ignis/Asset/AssetManager.h"
#include "Ignis/Script/ScriptBehaviour.h"
#include "Ignis/Renderer/Font.h"

namespace ignis
{
	void UISystem::OnUpdate(Scene& scene, uint32_t screen_w, uint32_t screen_h)
	{
		const glm::vec2 screen_min = { 0.0f, 0.0f };
		const glm::vec2 screen_max = { static_cast<float>(screen_w),
									   static_cast<float>(screen_h) };

		// Find all Canvas roots and resolve their subtrees
		auto canvas_view = scene.GetAllEntitiesWith<CanvasComponent, RectTransformComponent>();
		for (auto e_handle : canvas_view)
		{
			Entity canvas_entity = scene.GetEntityByHandle(e_handle);

			// Canvas root always fills the screen in ScreenSpace mode
			auto& canvas_comp = canvas_entity.GetComponent<CanvasComponent>();
			if (!canvas_comp.Visible) continue;

			auto& canvas_rect = canvas_entity.GetComponent<RectTransformComponent>();

			if (canvas_comp.Mode == CanvasComponent::RenderMode::ScreenSpace)
			{
				canvas_rect.ResolvedMin = screen_min;
				canvas_rect.ResolvedMax = screen_max;
			}
			else
			{
				// WorldSpace canvas: layout relative to its own OffsetMin/Max
				canvas_rect.ResolvedMin = canvas_rect.OffsetMin;
				canvas_rect.ResolvedMax = canvas_rect.OffsetMax;
			}

			// Recursively resolve children
			for (Entity child : canvas_entity.GetChildren())
				ResolveLayout(scene, child, canvas_rect.ResolvedMin, canvas_rect.ResolvedMax);
		}
	}

	void UISystem::ResolveLayout(Scene& scene, Entity node,
		const glm::vec2& parent_min,
		const glm::vec2& parent_max)
	{
		if (!node.IsValid()) return;
		if (!node.HasComponent<RectTransformComponent>()) return;

		auto& rect = node.GetComponent<RectTransformComponent>();
		glm::vec2 psize = parent_max - parent_min;

		glm::vec2 anchor_min_px = parent_min + rect.AnchorMin * psize;
		glm::vec2 anchor_max_px = parent_min + rect.AnchorMax * psize;

		rect.ResolvedMin = anchor_min_px + rect.OffsetMin;
		rect.ResolvedMax = anchor_max_px + rect.OffsetMax;

		for (Entity child : node.GetChildren())
			ResolveLayout(scene, child, rect.ResolvedMin, rect.ResolvedMax);
	}

	void UISystem::OnRender(Scene& scene, UIRenderer& ui_renderer,
		uint32_t screen_w, uint32_t screen_h)
	{
		// Collect canvases and sort by SortOrder
		std::vector<std::pair<int, Entity>> canvases;
		auto canvas_view = scene.GetAllEntitiesWith<CanvasComponent>();
		for (auto e_handle : canvas_view)
		{
			Entity e = scene.GetEntityByHandle(e_handle);
			auto& c = e.GetComponent<CanvasComponent>();
			if (c.Visible)
				canvases.emplace_back(c.SortOrder, e);
		}
		std::sort(canvases.begin(), canvases.end(),
			[](const auto& a, const auto& b) { return a.first < b.first; });

		for (auto& [sort_order, canvas_entity] : canvases)
		{
			float depth = 0.0f;
			for (Entity child : canvas_entity.GetChildren())
				RenderNode(scene, child, ui_renderer, sort_order, depth);
		}
	}

	void UISystem::RenderNode(Scene& scene, Entity node,
		UIRenderer& ui_renderer,
		int canvas_sort_order,
		float& depth_counter)
	{
		if (!node.IsValid()) return;
		if (!node.HasComponent<RectTransformComponent>()) return;

		const auto& rect = node.GetComponent<RectTransformComponent>();
		float my_depth = depth_counter++;

		if (node.HasComponent<ImageComponent>())
		{
			auto& img = node.GetComponent<ImageComponent>();
			if (img.Visible)
			{
				glm::vec4 color = img.Color;
				if (node.HasComponent<ButtonComponent>())
					color *= node.GetComponent<ButtonComponent>().CurrentColor;

				std::shared_ptr<Texture2D> tex;
				if (img.Texture)
					tex = AssetManager::GetAsset<Texture2D>(img.Texture);

				glm::vec2 draw_min = rect.ResolvedMin;
				glm::vec2 draw_max = rect.ResolvedMax;

				if (tex && img.Scale != ImageComponent::ScaleMode::Stretch)
				{
					float img_w = static_cast<float>(tex->GetWidth());
					float img_h = static_cast<float>(tex->GetHeight());
					float rect_w = rect.ResolvedMax.x - rect.ResolvedMin.x;
					float rect_h = rect.ResolvedMax.y - rect.ResolvedMin.y;
					float img_asp = img_w / img_h;
					float rect_asp = rect_w / rect_h;

					if (img.Scale == ImageComponent::ScaleMode::FitInside ||
						img.Scale == ImageComponent::ScaleMode::FitOutside)
					{
						bool fit_by_width = (img.Scale == ImageComponent::ScaleMode::FitInside)
							? (img_asp > rect_asp)
							: (img_asp < rect_asp);

						float new_w, new_h;
						if (fit_by_width) { new_w = rect_w; new_h = rect_w / img_asp; }
						else { new_h = rect_h; new_w = rect_h * img_asp; }

						float cx = rect.ResolvedMin.x + rect_w * 0.5f;
						float cy = rect.ResolvedMin.y + rect_h * 0.5f;
						draw_min = { cx - new_w * 0.5f, cy - new_h * 0.5f };
						draw_max = { cx + new_w * 0.5f, cy + new_h * 0.5f };
					}
					else if (img.Scale == ImageComponent::ScaleMode::NativeSize)
					{
						draw_min = rect.ResolvedMin;
						draw_max = { rect.ResolvedMin.x + img_w, rect.ResolvedMin.y + img_h };
					}
				}

				ui_renderer.SubmitRect(draw_min, draw_max,
					color, tex,
					canvas_sort_order, my_depth);
			}
		}

		if (node.HasComponent<ProgressBarComponent>())
		{
			auto& bar = node.GetComponent<ProgressBarComponent>();
			if (bar.Visible)
			{
				// Background
				ui_renderer.SubmitRect(rect.ResolvedMin, rect.ResolvedMax,
					bar.BackgroundColor, nullptr,
					canvas_sort_order, my_depth);

				// Foreground (fill)
				float t = bar.GetNormalizedValue();
				glm::vec2 fmin = rect.ResolvedMin;
				glm::vec2 fmax = rect.ResolvedMax;
				glm::vec2 size = rect.GetSize();

				switch (bar.Direction)
				{
				case ProgressBarComponent::FillDirection::LeftToRight:
					fmax.x = fmin.x + size.x * t; break;
				case ProgressBarComponent::FillDirection::RightToLeft:
					fmin.x = fmax.x - size.x * t; break;
				case ProgressBarComponent::FillDirection::TopToBottom:
					fmax.y = fmin.y + size.y * t; break;
				case ProgressBarComponent::FillDirection::BottomToTop:
					fmin.y = fmax.y - size.y * t; break;
				}

				ui_renderer.SubmitRect(fmin, fmax,
					bar.ForegroundColor, nullptr,
					canvas_sort_order, my_depth + 0.01f);
			}
		}

		if (node.HasComponent<UITextComponent>())
		{
			auto& text_comp = node.GetComponent<UITextComponent>();
			if (text_comp.Visible && !text_comp.Text.empty())
			{
				auto font = AssetManager::GetAsset<Font>(text_comp.Font);
				if (font)
				{
					ui_renderer.SubmitText(
						rect.ResolvedMin, rect.GetSize(),
						text_comp.Text, font,
						text_comp.Color, text_comp.FontSize,
						text_comp.HAlign, text_comp.VAlign,
						canvas_sort_order, my_depth + 0.02f);
				}
			}
		}

		for (Entity child : node.GetChildren())
			RenderNode(scene, child, ui_renderer, canvas_sort_order, depth_counter);
	}

	void UISystem::OnMouseMoved(Scene& scene, double x, double y)
	{
		m_mouse_pos = { static_cast<float>(x), static_cast<float>(y) };

		Entity hit;
		auto canvas_view = scene.GetAllEntitiesWith<CanvasComponent>();
		for (auto e_handle : canvas_view)
		{
			Entity canvas = scene.GetEntityByHandle(e_handle);
			for (Entity child : canvas.GetChildren())
			{
				if (HitTestNode(scene, child, m_mouse_pos, hit))
					break;
			}
		}

		// Update hover state
		if (hit != m_hovered_entity)
		{
			if (m_hovered_entity.IsValid())
			{
				if (m_hovered_entity.HasComponent<ButtonComponent>())
				{
					auto& btn = m_hovered_entity.GetComponent<ButtonComponent>();
					btn.IsHovered = false;
					btn.CurrentColor = btn.Interactable ? btn.NormalColor : btn.DisabledColor;
				}
				DispatchPointerExit(scene, m_hovered_entity);
			}

			m_hovered_entity = hit;

			if (m_hovered_entity.IsValid())
			{
				if (m_hovered_entity.HasComponent<ButtonComponent>())
				{
					auto& btn = m_hovered_entity.GetComponent<ButtonComponent>();
					btn.IsHovered = true;
					btn.CurrentColor = btn.Interactable ? btn.HoverColor : btn.DisabledColor;
				}
				DispatchPointerEnter(scene, m_hovered_entity);
			}
		}
	}

	void UISystem::OnMouseButtonPressed(Scene& scene, int button)
	{
		if (!m_hovered_entity.IsValid()) return;

		m_pressed_entity = m_hovered_entity;
		m_pressed_button = button;

		if (m_pressed_entity.HasComponent<ButtonComponent>())
		{
			auto& btn = m_pressed_entity.GetComponent<ButtonComponent>();
			if (btn.Interactable)
			{
				btn.IsPressed = true;
				btn.CurrentColor = btn.PressedColor;
			}
		}

		DispatchPointerDown(scene, m_pressed_entity, button);
	}

	void UISystem::OnMouseButtonReleased(Scene& scene, int button)
	{
		if (!m_pressed_entity.IsValid()) return;

		if (m_pressed_entity.HasComponent<ButtonComponent>())
		{
			auto& btn = m_pressed_entity.GetComponent<ButtonComponent>();
			btn.IsPressed = false;
			btn.CurrentColor = btn.IsHovered && btn.Interactable
				? btn.HoverColor
				: (btn.Interactable ? btn.NormalColor : btn.DisabledColor);
		}

		DispatchPointerUp(scene, m_pressed_entity, button);

		// Fire click only if still hovering over the same element
		if (m_pressed_entity == m_hovered_entity && m_pressed_button == button)
			DispatchPointerClick(scene, m_pressed_entity, button);

		m_pressed_entity = {};
		m_pressed_button = -1;
	}

	void UISystem::OnKeyTyped(Scene& scene, int keycode)
	{
		// Reserved for InputField - forward to focused entity in the future
		(void)scene; (void)keycode;
	}

	bool UISystem::HitTestNode(Scene& scene, Entity node,
		const glm::vec2& pos, Entity& out_hit)
	{
		if (!node.IsValid()) return false;
		if (!node.HasComponent<RectTransformComponent>()) return false;

		const auto& rect = node.GetComponent<RectTransformComponent>();

		// Test children first (they're rendered on top)
		std::vector<Entity> children = node.GetChildren();
		for (auto it = children.rbegin(); it != children.rend(); ++it)
		{
			if (HitTestNode(scene, *it, pos, out_hit))
				return true;
		}

		// Check this node
		bool is_raycast_target =
			(node.HasComponent<ImageComponent>() && node.GetComponent<ImageComponent>().RaycastTarget)
			|| node.HasComponent<ButtonComponent>();

		if (!is_raycast_target) return false;

		if (pos.x >= rect.ResolvedMin.x && pos.x <= rect.ResolvedMax.x &&
			pos.y >= rect.ResolvedMin.y && pos.y <= rect.ResolvedMax.y)
		{
			out_hit = node;
			return true;
		}

		return false;
	}

	void UISystem::DispatchPointerEnter(Scene& scene, Entity entity)
	{
		if (auto* sb = scene.GetRuntimeScript(entity.GetID()))
			sb->OnPointerEnter();
	}

	void UISystem::DispatchPointerExit(Scene& scene, Entity entity)
	{
		if (auto* sb = scene.GetRuntimeScript(entity.GetID()))
			sb->OnPointerExit();
	}

	void UISystem::DispatchPointerDown(Scene& scene, Entity entity, int btn)
	{
		if (auto* sb = scene.GetRuntimeScript(entity.GetID()))
			sb->OnPointerDown(btn);
	}

	void UISystem::DispatchPointerUp(Scene& scene, Entity entity, int btn)
	{
		if (auto* sb = scene.GetRuntimeScript(entity.GetID()))
			sb->OnPointerUp(btn);
	}

	void UISystem::DispatchPointerClick(Scene& scene, Entity entity, int btn)
	{
		if (auto* sb = scene.GetRuntimeScript(entity.GetID()))
			sb->OnPointerClick(btn);
	}
}