#pragma once

#include <entt.hpp>

#include "Scene.h"
#include "Components.h"

namespace ignis
{
	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity handle, entt::registry* registry);
		~Entity() = default;
		
		bool operator==(const Entity& other) const
		{
			return m_handle == other.m_handle && m_registry == other.m_registry;
		}
		
		operator bool() const { return m_handle != entt::null && m_registry != nullptr; }
		operator uint32_t() const { return static_cast<uint32_t>(m_handle); }

		template<typename T, typename... Args>
			requires std::is_base_of_v<Component, T>
		T& AddComponent(Args&&... args)
		{
			return m_registry->emplace<T>(m_handle, std::forward<Args>(args)...);
		}
		
		template<typename T>
			requires std::is_base_of_v<Component, T>
		T& GetComponent()
		{
			return m_registry->get<T>(m_handle);
		}

		template<typename T>
			requires std::is_base_of_v<Component, T>
		bool HasComponent()
		{
			return m_registry->all_of<T>(m_handle);
		}

		template<typename T>
			requires std::is_base_of_v<Component, T>
		void RemoveComponent()
		{
			m_registry->remove<T>(m_handle);
		}

	private:
		entt::entity m_handle = entt::null;
		entt::registry* m_registry = nullptr;
	};
}