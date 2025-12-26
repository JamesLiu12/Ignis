#pragma once

#include "DebugPanel.h"
#include "Ignis/Physics/PhysicsWorld.h"
#include <memory>

namespace ignis {

	class PhysicsDebugPanel : public DebugPanel
	{
	public:
		PhysicsDebugPanel() = default;
		~PhysicsDebugPanel() = default;

		// EditorPanel interface
		void OnImGuiRender() override;
		std::string_view GetName() const override { return "Physics Debug"; }
		std::string_view GetID() const override { return "PhysicsDebug"; }

		// Legacy method for direct PhysicsWorld access (for backward compatibility)
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