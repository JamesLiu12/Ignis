#pragma once

#include "Ignis/Core/API.h"
#include "Ignis/Scene/Entity.h"

namespace ignis
{
	class IGNIS_API ScriptBehaviour
	{
	public:
		virtual ~ScriptBehaviour() = default;

		virtual void OnCreate() {}
		virtual void OnUpdate(float dt) {}
		virtual void OnDestroy() {}

		void SetEntity(Entity e) { m_entity = e; }
		Entity GetEntity() const { return m_entity; }

	private:
		Entity m_entity;
	};
}