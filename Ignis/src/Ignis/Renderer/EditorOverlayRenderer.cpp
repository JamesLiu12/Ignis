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
}