#pragma once

#include "Ignis/Core/API.h"
#include "Ignis/Scene/Entity.h"

namespace ignis
{
	class IGNIS_API ScriptBehaviour
	{
	public:
		virtual ~ScriptBehaviour() = default;

		// 3D lifecycle
		virtual void OnCreate() {}
		virtual void OnUpdate(float dt) {}
		virtual void OnDestroy() {}

		// UI events
		virtual void OnPointerEnter() {}
		virtual void OnPointerExit() {}
		virtual void OnPointerDown(int btn) {}
		virtual void OnPointerUp(int btn) {}
		virtual void OnPointerClick(int btn) {}

		void SetEntity(Entity e) { m_entity = e; }
		Entity GetEntity() const { return m_entity; }
		Scene* GetScene() const { return m_entity.GetScene(); }

	private:
		Entity m_entity;
	};
}