#pragma once

namespace ignis
{
	template<typename T, typename... Args>
		requires std::is_base_of_v<Component, T>
	T& Entity::AddComponent(Args&&... args)
	{
		return m_scene->m_registry.emplace<T>(m_handle, std::forward<Args>(args)...);
	}

	template<typename T>
		requires std::is_base_of_v<Component, T>
	T& Entity::GetComponent() const
	{
		return m_scene->m_registry.get<T>(m_handle);
	}

	template<typename T>
		requires std::is_base_of_v<Component, T>
	bool Entity::HasComponent()
	{
		return m_scene->m_registry.all_of<T>(m_handle);
	}

	template<typename T>
		requires std::is_base_of_v<Component, T>
	void Entity::RemoveComponent()
	{
		m_scene->m_registry.remove<T>(m_handle);
	}
}