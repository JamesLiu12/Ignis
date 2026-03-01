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

		template<std::derived_from<Component> T, typename... Args>
		T& AddComponent(Args&&... args);
		
		template<std::derived_from<Component> T>
		T& GetComponent() const;

		template<std::derived_from<Component> T>
		bool HasComponent() const;

		template<std::derived_from<Component> T>
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
		
		template<std::invocable<Entity> Func>
		void ForEachChild(Func func);

		void MoveTo(Entity target_prev_sibling);
		void SetSiblingIndex(int index);
		void MoveToAfter(Entity target_prev_sibling);

		glm::mat4 GetLocalTransform() const;
		glm::mat4 GetWorldTransform() const;

	private:
		entt::entity m_handle = entt::null;
		Scene* m_scene = nullptr;
	};
}