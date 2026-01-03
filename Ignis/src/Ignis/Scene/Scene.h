#pragma once

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

	class Entity;

	class Scene
	{
	public:
		Scene() = default;
		~Scene() = default;

		Scene(const Scene&) = delete;
		Scene& operator=(const Scene&) = delete;

		Scene(Scene&&) = default;
		Scene& operator=(Scene&&) = default;

		Entity CreateEntity(const std::string name = "");

		void OnRender();

	private:
		entt::registry m_registry;
		LightEnvironment m_light_environment;
		Environment m_scene_environment;
		EnvironmentSettings m_environment_settings;

		// TODO: Move this to SceneRenderer
		friend class GLRenderer;
	};
}