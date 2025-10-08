#pragma once

#include "Ignis/Physics/PhysicsWorld.h"
#include <memory>

namespace ignis {

	class PhysicsDebugPanel
	{
	public:
		PhysicsDebugPanel() = default;
		~PhysicsDebugPanel() = default;

		void OnImGuiRender(PhysicsWorld* world, bool& is_open);

	private:
		void ShowBodyInfo(PhysicsBody* body, int index);
		void ShowWorldInfo(PhysicsWorld* world);
		void ShowControls(PhysicsWorld* world);

		bool m_show_details = true;
		int m_selected_body = -1;
	};

}