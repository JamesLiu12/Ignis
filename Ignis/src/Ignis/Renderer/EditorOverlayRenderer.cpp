#include "EditorOverlayRenderer.h"
#include "Ignis/Scene/Components.h"

namespace ignis
{
	EditorOverlayRenderer::EditorOverlayRenderer(DebugRenderer& debug_renderer)
		: m_debug_renderer(debug_renderer)
	{
	}

	void EditorOverlayRenderer::BeginScene(std::shared_ptr<Camera> camera)
	{
		m_debug_renderer.BeginScene(camera);
	}

	void EditorOverlayRenderer::Flush()
	{
		m_debug_renderer.Flush();
	}

	void EditorOverlayRenderer::DrawColliders(Entity entity, const glm::vec4& color)
	{
		if (!entity || !entity.IsValid())
			return;

		const glm::mat4 world = entity.GetWorldTransform();

		if (entity.HasComponent<BoxColliderComponent>())
			DrawBoxCollider(world, entity.GetComponent<BoxColliderComponent>(), color);

		if (entity.HasComponent<SphereColliderComponent>())
			DrawSphereCollider(world, entity.GetComponent<SphereColliderComponent>(), color);

		if (entity.HasComponent<CapsuleColliderComponent>())
			DrawCapsuleCollider(world, entity.GetComponent<CapsuleColliderComponent>(), color);
	}

	void EditorOverlayRenderer::DrawAllColliders(Scene& scene, const glm::vec4& color)
	{
		for (auto handle : scene.GetAllEntitiesWith<TransformComponent>())
		{
			Entity entity = scene.GetEntityByHandle(handle);
			DrawColliders(entity, color);
		}
	}

	void EditorOverlayRenderer::DrawBoxCollider(const glm::mat4& world,
		const BoxColliderComponent& c,
		const glm::vec4& color)
	{
		m_debug_renderer.DrawWireBox(world, c.HalfSize, c.Offset, color);
	}

	void EditorOverlayRenderer::DrawSphereCollider(const glm::mat4& world,
		const SphereColliderComponent& c,
		const glm::vec4& color)
	{
		m_debug_renderer.DrawWireSphere(world, c.Radius, c.Offset, color);
	}

	void EditorOverlayRenderer::DrawCapsuleCollider(const glm::mat4& world,
		const CapsuleColliderComponent& c,
		const glm::vec4& color)
	{
		m_debug_renderer.DrawWireCapsule(world, c.Radius, c.HalfHeight, c.Offset, color);
	}

	void EditorOverlayRenderer::DrawTransformGizmo(Entity entity, GizmoMode mode)
	{
		if (!entity || !entity.IsValid() || mode == GizmoMode::None)
			return;

		switch (mode)
		{
		case GizmoMode::Translate: DrawTranslateGizmo(entity); break;
		case GizmoMode::Rotate:    DrawRotateGizmo(entity);    break;
		case GizmoMode::Scale:     DrawScaleGizmo(entity);     break;
		default: break;
		}
	}

	void EditorOverlayRenderer::DrawTranslateGizmo(Entity entity)
	{
		const glm::mat4 world = entity.GetWorldTransform();
		const glm::vec3 origin = glm::vec3(world[3]);
		const glm::vec3 right = glm::normalize(glm::vec3(world[0]));
		const glm::vec3 up = glm::normalize(glm::vec3(world[1]));
		const glm::vec3 forward = glm::normalize(glm::vec3(world[2]));

		constexpr float kLength = 1.5f;
		constexpr float kHeadSize = 0.15f;

		m_debug_renderer.DrawArrow(origin, right, kLength, kHeadSize, kGizmoColorX);
		m_debug_renderer.DrawArrow(origin, up, kLength, kHeadSize, kGizmoColorY);
		m_debug_renderer.DrawArrow(origin, forward, kLength, kHeadSize, kGizmoColorZ);
	}

	void EditorOverlayRenderer::DrawRotateGizmo(Entity entity)
	{
		const glm::mat4 world = entity.GetWorldTransform();
		const glm::vec3 center = glm::vec3(world[3]);
		const glm::vec3 right = glm::normalize(glm::vec3(world[0]));
		const glm::vec3 up = glm::normalize(glm::vec3(world[1]));
		const glm::vec3 forward = glm::normalize(glm::vec3(world[2]));

		constexpr float kRadius = 1.5f;

		m_debug_renderer.DrawCircle(center, right, kRadius, kGizmoColorX);
		m_debug_renderer.DrawCircle(center, up, kRadius, kGizmoColorY);
		m_debug_renderer.DrawCircle(center, forward, kRadius, kGizmoColorZ);
	}

	void EditorOverlayRenderer::DrawScaleGizmo(Entity entity)
	{
		const glm::mat4 world = entity.GetWorldTransform();
		const glm::vec3 origin = glm::vec3(world[3]);
		const glm::vec3 right = glm::normalize(glm::vec3(world[0]));
		const glm::vec3 up = glm::normalize(glm::vec3(world[1]));
		const glm::vec3 forward = glm::normalize(glm::vec3(world[2]));

		constexpr float kLength = 1.5f;
		constexpr float kBoxHalf = 0.08f;

		m_debug_renderer.DrawLine(origin, origin + right * kLength, kGizmoColorX);
		m_debug_renderer.DrawLine(origin, origin + up * kLength, kGizmoColorY);
		m_debug_renderer.DrawLine(origin, origin + forward * kLength, kGizmoColorZ);

		m_debug_renderer.DrawAABB(origin + right * kLength, glm::vec3(kBoxHalf), kGizmoColorX);
		m_debug_renderer.DrawAABB(origin + up * kLength, glm::vec3(kBoxHalf), kGizmoColorY);
		m_debug_renderer.DrawAABB(origin + forward * kLength, glm::vec3(kBoxHalf), kGizmoColorZ);

		m_debug_renderer.DrawAABB(origin, glm::vec3(kBoxHalf * 1.4f), kGizmoColorUniform);
	}
}