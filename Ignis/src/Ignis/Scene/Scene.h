#pragma once

#include <entt.hpp>

namespace ignis
{
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

	private:
		entt::registry m_registry;
	};
}