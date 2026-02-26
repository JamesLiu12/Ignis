#pragma once

#include "Entity.h"
#include "Ignis/Renderer/Environment.h"

#include <entt.hpp>
#include <glm/glm.hpp>

namespace ignis
{
	struct DirectionalLight
	{
		glm::vec3 Direction;
		glm::vec3 Radiance;
	};

	struct PointLight
	{
		glm::vec3 Position;
		glm::vec3 Radiance;
		float Constant;
		float Linear;
		float Quadratic;
	};

	struct SpotLight
	{
		glm::vec3 Position;
		glm::vec3 Radiance;
		glm::vec3 Direction;
		float Constant;
		float Linear;
		float Quadratic;
		float CutOff;
		float OuterCutOff;
	};

	struct LightEnvironment
	{
		static constexpr size_t MaxDirectionalLights = 4;
		static constexpr size_t MaxPointLights = 16;
		static constexpr size_t MaxSpotLights = 16;

		std::vector<DirectionalLight> DirectionalLights;
		std::vector<PointLight> PointLights;
		std::vector<SpotLight> SpotLights;
	};

	class SceneRenderer;

	class Scene
	{
	public:
		Scene(std::string_view name = "UntitledScene")
			: m_name(name) {}
		~Scene() = default;

		Scene(const Scene&) = delete;
		Scene& operator=(const Scene&) = delete;

		Scene(Scene&&) = default;
		Scene& operator=(Scene&&) = default;

		Entity CreateEntity(const std::string name = "");
		Entity CreateEntity(Entity parent, const std::string name = "");
		Entity CreateEntityWithID(UUID uuid, const std::string& name = "");
		Entity CreateEntityWithID(UUID uuid, Entity parent, const std::string& name = "");

		void OnRender(const SceneRenderer& scene_renderer);
	
		template<typename... Components>
		auto GetAllEntitiesWith()
		{
			return m_registry.view<Components...>();
		}

		// Get Entity wrapper from handle (for editor)
		Entity GetEntityByHandle(entt::entity handle);

		Entity GetEntityByID(UUID id) const;

		const std::string& GetName() const { return m_name; }
		void SetName(std::string_view name) { m_name = name; }

	private:
		entt::registry m_registry;
		LightEnvironment m_light_environment;
		Environment m_scene_environment;
		EnvironmentSettings m_environment_settings;
		std::unordered_map<UUID, Entity> m_id_entity_map;
		std::string m_name;

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneRenderer;
	};
}

#include "EntityImpl.h"