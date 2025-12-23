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
		void OnImGuiRender(bool& is_open) override;
		const char* GetName() const override { return "Physics Debug"; }
		const char* GetID() const override { return "PhysicsDebug"; }

		// Legacy method for direct PhysicsWorld access (will be refactored in Step 8)
		void OnImGuiRender(PhysicsWorld* world, bool& is_open);

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