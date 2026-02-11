#pragma once

#include "Ignis.h"

namespace ignis {

class Sandbox : public Application
{
public:
	Sandbox();
	~Sandbox();

private:
	void CreatePhysicsTestScene();
	std::unique_ptr<PhysicsWorld> m_physics_world;
};

} // namespace ignis