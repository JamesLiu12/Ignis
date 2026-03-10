#pragma once

#include "Ignis/Core/API.h"
#include "Ignis/Scene/Entity.h"
#include "Ignis/Scene/Scene.h"
#include "DebugRenderer.h"

#include <glm/glm.hpp>
#include <memory>

namespace ignis
{
	enum class GizmoMode
	{
		None,
		Translate,
		Rotate,
		Scale
	};

	class IGNIS_API EditorOverlayRenderer
	{
	public:
		explicit EditorOverlayRenderer(DebugRenderer& debug_renderer);

		void BeginScene(std::shared_ptr<Camera> camera);

		void Flush();

		void DrawColliders(Entity entity,
			const glm::vec4& color = kDefaultColliderColor);

		void DrawAllColliders(Scene& scene,
			const glm::vec4& color = kDefaultColliderColor);

		void DrawTransformGizmo(Entity entity, GizmoMode mode);

	private:
		void DrawBoxCollider(const glm::mat4& world, const BoxColliderComponent& c, const glm::vec4& color);
		void DrawSphereCollider(const glm::mat4& world, const SphereColliderComponent& c, const glm::vec4& color);
		void DrawCapsuleCollider(const glm::mat4& world, const CapsuleColliderComponent& c, const glm::vec4& color);

		void DrawTranslateGizmo(Entity entity);
		void DrawRotateGizmo(Entity entity);
		void DrawScaleGizmo(Entity entity);

		DebugRenderer& m_debug_renderer;

		static constexpr glm::vec4 kDefaultColliderColor = { 0.13f, 0.92f, 0.40f, 1.0f };
		static constexpr glm::vec4 kGizmoColorX = { 0.90f, 0.20f, 0.20f, 1.0f };
		static constexpr glm::vec4 kGizmoColorY = { 0.20f, 0.90f, 0.20f, 1.0f };
		static constexpr glm::vec4 kGizmoColorZ = { 0.20f, 0.40f, 0.90f, 1.0f };
		static constexpr glm::vec4 kGizmoColorUniform = { 0.90f, 0.80f, 0.20f, 1.0f };
	};
}