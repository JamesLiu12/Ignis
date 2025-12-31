#include "Scene.h"
#include "Entity.h"

namespace ignis
{
	Entity Scene::CreateEntity(const std::string name)
	{
		Entity entity = Entity(m_registry.create(), &m_registry);
		entity.AddComponent<TransformComponent>();
		entity.AddComponent<TagComponent>(name.empty() ? "Entity" : name);
		return entity;
	}

	static inline void ComputeAttenuationFromRange(float range, float& out_linear, float& out_quadratic, float edge = 0.01f)
	{
		range = std::max(range, 1e-4f);
		edge = std::clamp(edge, 1e-6f, 1.0f);

		out_linear = 0.0f;
		out_quadratic = (1.0f / edge - 1.0f) / (range * range);
	}

	void Scene::OnRender()
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
	}
}