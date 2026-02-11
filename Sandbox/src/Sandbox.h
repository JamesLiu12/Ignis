#pragma once

#include "Ignis.h"
#include "Ignis/Physics/PhysicsWorld.h"
#include "Ignis/Physics/PhysicsTypes.h"

class Sandbox : public ignis::Application
{
public:
	Sandbox();
	~Sandbox();

private:
	void CreatePhysicsTestScene();
	std::unique_ptr<ignis::PhysicsWorld> m_physics_world;
};