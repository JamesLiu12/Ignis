#include "Scene.h"
#include "Entity.h"
#include "Ignis/Asset/AssetManager.h"
#include "Ignis/Renderer/SceneRenderer.h"
#include "Ignis/Script/ScriptBehaviour.h"
#include "Ignis/Script/Script.h"
#include "Ignis/Script/ScriptRegistry.h"

namespace ignis
{
	Entity Scene::CreateEntity(const std::string name)
	{
		return CreateEntity({}, name);
	}

	Entity Scene::CreateEntity(Entity parent, const std::string name)
	{
		Entity entity = Entity(m_registry.create(), this);

		auto& id_component = entity.AddComponent<IDComponent>(UUID());
		m_id_entity_map[id_component.ID] = entity;

		entity.AddComponent<RelationshipComponent>();

		entity.AddComponent<TransformComponent>();
		entity.AddComponent<TagComponent>(name.empty() ? "Entity" : name);

		if (parent.IsValid())
		{
			entity.SetParent(parent);
		}

		return entity;
	}

	Entity Scene::CreateEntityWithID(UUID uuid, const std::string& name)
	{
		return CreateEntityWithID(uuid, {}, name);
	}

	Entity Scene::CreateEntityWithID(UUID uuid, Entity parent, const std::string& name)
	{
		Entity entity = Entity(m_registry.create(), this);

		auto& id_component = entity.AddComponent<IDComponent>(uuid);
		m_id_entity_map[id_component.ID] = entity;

		entity.AddComponent<RelationshipComponent>();

		entity.AddComponent<TransformComponent>();
		entity.AddComponent<TagComponent>(name.empty() ? "Entity" : name);

		if (parent.IsValid())
		{
			entity.SetParent(parent);
		}

		return entity;
	}

	Entity Scene::GetEntityByHandle(entt::entity handle)
	{
		return Entity(handle, this);
	}
	
	Entity Scene::GetEntityByID(UUID id) const
	{
		const auto it = m_id_entity_map.find(id);
		if (it != m_id_entity_map.end()) return it->second;
		return {};
	}

	static inline void ComputeAttenuationFromRange(float range, float& out_linear, float& out_quadratic, float edge = 0.01f)
	{
		range = std::max(range, 1e-4f);
		edge = std::clamp(edge, 1e-6f, 1.0f);

		out_linear = 0.0f;
		out_quadratic = (1.0f / edge - 1.0f) / (range * range);
	}

	void Scene::OnRender(const SceneRenderer& scene_renderer)
	{
		m_light_environment = LightEnvironment();

		// -------------------------
		// Directional
		// -------------------------
		{
			uint32_t light_index = 0;
			auto lights = m_registry.group<DirectionalLightComponent>(entt::get<TransformComponent>);

			lights.each([&](auto entity, DirectionalLightComponent& light, TransformComponent& transform)
				{
					if (light_index >= LightEnvironment::MaxDirectionalLights) return;
					
					glm::vec3 direction = glm::normalize(transform.GetRotationQuat() * glm::vec3(0.0f, 0.0f, -1.0f));

					m_light_environment.DirectionalLights.emplace_back
					(
						direction,
						light.Color * light.Intensity
					);
					light_index++;
				});
		}

		// -------------------------
		// Point
		// -------------------------
		{
			uint32_t light_index = 0;
			auto lights = m_registry.group<PointLightComponent>(entt::get<TransformComponent>);

			lights.each([&](auto /*entity*/, PointLightComponent& light, TransformComponent& transform)
				{
					if (light_index >= LightEnvironment::MaxPointLights) return;

					float linear = 0.0f;
					float quadratic = 0.0f;
					ComputeAttenuationFromRange(light.Range, linear, quadratic);

					m_light_environment.PointLights.emplace_back
					(
						transform.Translation,
						light.Color * light.Intensity,
						1.0f,
						linear,
						quadratic
					);
					light_index++;
				});
		}

		// -------------------------
		// Spot
		// -------------------------
		{
			uint32_t light_index = 0;
			auto lights = m_registry.group<SpotLightComponent>(entt::get<TransformComponent>);

			lights.each([&](auto entity, SpotLightComponent& light, TransformComponent& transform)
				{
					if (light_index >= LightEnvironment::MaxSpotLights) return;

					float linear = 0.0f;
					float quadratic = 0.0f;
					ComputeAttenuationFromRange(light.Range, linear, quadratic);

					float inner = light.InnerConeAngle;
					float outer = light.OuterConeAngle;
					if (inner > outer) std::swap(inner, outer);

					glm::vec3 direction = glm::normalize(transform.GetRotationQuat() * glm::vec3(0.0f, 0.0f, -1.0f));

					m_light_environment.SpotLights.emplace_back
					(
						transform.Translation,
						light.Color * light.Intensity,
						direction,
						1.0f,
						linear,
						quadratic,
						glm::cos(glm::radians(inner)),
						glm::cos(glm::radians(outer))
					);
					light_index++;
				});
		}

		// -------------------------
		// SkyLight
		// -------------------------
		{
			auto sky_lights = m_registry.group<SkyLightComponent>();

			sky_lights.each([&](auto entity, SkyLightComponent& sky_light)
				{
					m_scene_environment = AssetManager::GetAsset<Environment>(sky_light.SceneEnvironment),
					m_environment_settings.Intensity = sky_light.Intensity;
					m_environment_settings.Rotation = sky_light.Rotation;
					m_environment_settings.Tint = sky_light.Tint;
					m_environment_settings.SkyboxLod = sky_light.SkyboxLod;
				});
		}

		// -------------------------
		// Mesh
		// -------------------------
		{
			auto meshes = m_registry.group<MeshComponent>(entt::get<TransformComponent>);

			meshes.each([&](auto entity_handle, MeshComponent& mesh_component, TransformComponent& transform)
				{
					if (auto mesh = AssetManager::GetAsset<Mesh>(mesh_component.Mesh))
					{
						mesh->SetMaterialData(0, mesh_component.MeterialData);
						Entity entity(entity_handle, this);
						scene_renderer.SubmitMesh(*mesh, entity.GetWorldTransform());
					}
				});
		}

		scene_renderer.SubmitSkybox();
	}

	void Scene::OnRuntimeStart()
	{
		OnRuntimeStop();

		auto scripts = m_registry.view<ScriptComponent>();

		scripts.each([&](entt::entity entity_handle, ScriptComponent script_component)
			{
				Entity entity(entity_handle, this);

				if (!script_component.Enabled)
					return;

				if (script_component.Script == AssetHandle::Invalid)
					return;

				auto script = AssetManager::GetAsset<Script>(script_component.Script);
				if (!script)
				{
					Log::CoreWarn("[Scene::OnRuntimeStart] Invalid ScriptAsset on entity {}", entity.GetID().ToString());
					return;
				}

				script->GetBehaviour().SetEntity(entity);
				script->GetBehaviour().OnCreate();
			});
	}

	void Scene::OnRuntimeUpdate(float dt)
	{
		auto scripts = m_registry.view<ScriptComponent>();

		scripts.each([&](entt::entity entity_handle, ScriptComponent script_component)
			{
				if (!script_component.Enabled)
					return;

				auto script = AssetManager::GetAsset<Script>(script_component.Script);

				script->GetBehaviour().OnUpdate(dt);
			});
	}

	void Scene::OnRuntimeStop()
	{
		auto scripts = m_registry.view<ScriptComponent>();

		scripts.each([&](entt::entity entity_handle, ScriptComponent script_component)
			{
				if (!script_component.Enabled)
					return;

				auto script = AssetManager::GetAsset<Script>(script_component.Script);

				script->GetBehaviour().OnDestroy();
			});
	}
}