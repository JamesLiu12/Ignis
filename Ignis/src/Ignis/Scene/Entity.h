#pragma once

#include <entt.hpp>

#include "Components.h"

namespace ignis
{
	class Scene;

	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity handle, Scene* scene);
		~Entity() = default;

		template<typename T, typename... Args>
			requires std::is_base_of_v<Component, T>
		T& AddComponent(Args&&... args);
		
		template<typename T>
			requires std::is_base_of_v<Component, T>
		T& GetComponent() const;

		template<typename T>
			requires std::is_base_of_v<Component, T>
		bool HasComponent();

		template<typename T>
			requires std::is_base_of_v<Component, T>
		void RemoveComponent();

		bool IsValid() const;

		bool operator==(const Entity& other) const
		{
			return m_handle == other.m_handle && m_scene == other.m_scene;
		}

		operator bool() const;
		operator uint32_t() const { return static_cast<uint32_t>(m_handle); }

		UUID GetID() const;
		UUID GetParentID() const;
		Entity GetParent() const;
		void SetParent(Entity new_parent);
		void Unparent();
		void AddChild(Entity child);
		void RemoveChild(Entity child);
		std::vector<Entity> GetChildren() const;
		
		template<typename Func>
			requires std::invocable<Func, Entity>
		void ForEachChild(Func func);

		void MoveTo(Entity target_prev_sibling);
		void SetSiblingIndex(int index);
		void MoveToAfter(Entity target_prev_sibling);

	private:
		entt::entity m_handle = entt::null;
		Scene* m_scene = nullptr;
	};
}