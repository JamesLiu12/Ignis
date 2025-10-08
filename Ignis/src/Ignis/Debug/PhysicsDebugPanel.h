#pragma once

#include "Ignis/Physics/PhysicsWorld.h"
#include <memory>

namespace ignis {

	class PhysicsDebugPanel
	{
	public:
		PhysicsDebugPanel() = default;
		~PhysicsDebugPanel() = default;

		void OnImGuiRender(PhysicsWorld* world, bool& isOpen);

	private:
		void ShowBodyInfo(PhysicsBody* body, int index);
		void ShowWorldInfo(PhysicsWorld* world);
		void ShowControls(PhysicsWorld* world);

		bool m_showDetails = true;
		int m_selectedBody = -1;
	};

}