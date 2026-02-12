#pragma once

#include "Ignis.h"
#include "Panels/EditorPanel.h"

namespace ignis {

	class PhysicsDebugPanel : public EditorPanel
	{
	public:
		PhysicsDebugPanel() = default;
		~PhysicsDebugPanel() = default;

		// EditorPanel interface
		void OnImGuiRender() override;
		std::string_view GetName() const override { return "Physics Debug"; }
		std::string_view GetID() const override { return "PhysicsDebug"; }

		void OnImGuiRender(PhysicsWorld* world);

		// Set the physics world reference for the panel
		void SetPhysicsWorld(PhysicsWorld* world) { m_physics_world = world; }

	private:
		void ShowBodyInfo(PhysicsBody* body, int index);
		void ShowWorldInfo(PhysicsWorld* world);
		void ShowControls(PhysicsWorld* world);

		PhysicsWorld* m_physics_world = nullptr;
		bool m_show_details = true;
		int m_selected_body = -1;
	};

}