#pragma once

#include "Ignis/Core/API.h"
#include "UIComponents.h"
#include "UIRenderer.h"
#include "Ignis/Scene/Entity.h"

#include <glm/glm.hpp>

namespace ignis
{
	class Scene;

	class IGNIS_API UISystem
	{
	public:
		UISystem() = default;
		~UISystem() = default;

		// Resolves all RectTransform layouts. Call before OnRender.
		void OnUpdate(Scene& scene, uint32_t screen_width, uint32_t screen_height);

		// Submits UI geometry to UIRenderer. Call after Scene::OnRender.
		void OnRender(Scene& scene, UIRenderer& ui_renderer,
			uint32_t screen_width, uint32_t screen_height);

		// Runtime-only event dispatch
		void OnMouseMoved(Scene& scene, double x, double y);
		void OnMouseButtonPressed(Scene& scene, int button);
		void OnMouseButtonReleased(Scene& scene, int button);
		void OnKeyTyped(Scene& scene, int keycode);

	private:
		void ResolveLayout(Scene& scene, Entity node,
			const glm::vec2& parent_min,
			const glm::vec2& parent_max);

		void RenderNode(Scene& scene, Entity node,
			UIRenderer& ui_renderer,
			int canvas_sort_order,
			float& depth_counter);

		bool HitTestNode(Scene& scene, Entity node,
			const glm::vec2& pos, Entity& out_hit);

		void DispatchPointerEnter(Scene& scene, Entity entity);
		void DispatchPointerExit(Scene& scene, Entity entity);
		void DispatchPointerDown(Scene& scene, Entity entity, int btn);
		void DispatchPointerUp(Scene& scene, Entity entity, int btn);
		void DispatchPointerClick(Scene& scene, Entity entity, int btn);

		glm::vec2 m_mouse_pos = { 0.0f, 0.0f };
		Entity    m_hovered_entity;
		Entity    m_pressed_entity;
		int       m_pressed_button = -1;
	};
}