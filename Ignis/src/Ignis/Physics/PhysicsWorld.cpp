#include "PhysicsWorld.h"
#include "Ignis/Core/Log.h"

namespace ignis {

	PhysicsWorld::PhysicsWorld()
		: m_collision_configuration(nullptr)
		, m_dispatcher(nullptr)
		, m_broadphase(nullptr)
		, m_solver(nullptr)
		, m_dynamics_world(nullptr)
		, m_initialized(false)
	{
	}

	PhysicsWorld::~PhysicsWorld()
	{
		Shutdown();
	}

	void PhysicsWorld::Init()
	{
		if (m_initialized)
		{
			Log::CoreWarn("PhysicsWorld already initialized!");
			return;
		}

		// Step 1: Create collision configuration
		m_collision_configuration = new btDefaultCollisionConfiguration();

		// Step 2: Create collision dispatcher
		m_dispatcher = new btCollisionDispatcher(m_collision_configuration);

		// Step 3: Create broadphase (spatial optimization)
		m_broadphase = new btDbvtBroadphase();

		// Step 4: Create constraint solver
		m_solver = new btSequentialImpulseConstraintSolver();

		// Step 5: Create dynamics world
		m_dynamics_world = new btDiscreteDynamicsWorld(
			m_dispatcher,
			m_broadphase,
			m_solver,
			m_collision_configuration
		);

		// Step 6: Set default gravity
		m_dynamics_world->setGravity(btVector3(0.0f, -9.81f, 0.0f));

		m_initialized = true;
		Log::CoreInfo("PhysicsWorld initialized successfully");
	}

	void PhysicsWorld::Shutdown()
	{
		if (!m_initialized)
			return;

		// Remove all bodies
		m_bodies.clear();

		// Delete dynamics world
		if (m_dynamics_world)
		{
			// Remove all remaining collision objects
			for (int i = m_dynamics_world->getNumCollisionObjects() - 1; i >= 0; i--)
			{
				btCollisionObject* obj = m_dynamics_world->getCollisionObjectArray()[i];
				btRigidBody* body = btRigidBody::upcast(obj);
				if (body && body->getMotionState())
				{
					delete body->getMotionState();
				}
				m_dynamics_world->removeCollisionObject(obj);
				delete obj;
			}

			delete m_dynamics_world;
			m_dynamics_world = nullptr;
		}

		// Delete solver
		delete m_solver;
		m_solver = nullptr;

		// Delete broadphase
		delete m_broadphase;
		m_broadphase = nullptr;

		// Delete dispatcher
		delete m_dispatcher;
		m_dispatcher = nullptr;

		// Delete collision configuration
		delete m_collision_configuration;
		m_collision_configuration = nullptr;

		m_initialized = false;
		Log::CoreInfo("PhysicsWorld shutdown complete");
	}

	void PhysicsWorld::Step(float delta_time)
	{
		if (!m_initialized || !m_dynamics_world)
			return;

		// Step the simulation
		m_dynamics_world->stepSimulation(delta_time, 10);
	}

	std::shared_ptr<PhysicsBody> PhysicsWorld::CreateBody(const RigidBodyDesc& desc)
	{
		if (!m_initialized || !m_dynamics_world)
		{
			Log::CoreError("Cannot create body: PhysicsWorld not initialized!");
			return nullptr;
		}

		auto body = std::make_shared<PhysicsBody>(desc, m_dynamics_world);
		m_bodies.push_back(body);

		Log::CoreTrace("Created physics body (total: {})", m_bodies.size());
		return body;
	}

	void PhysicsWorld::RemoveBody(std::shared_ptr<PhysicsBody> body)
	{
		if (!body)
			return;

		auto it = std::find(m_bodies.begin(), m_bodies.end(), body);
		if (it != m_bodies.end())
		{
			m_bodies.erase(it);
			Log::CoreTrace("Removed physics body (remaining: {})", m_bodies.size());
		}
	}

	void PhysicsWorld::SetGravity(const glm::vec3& gravity)
	{
		if (m_dynamics_world)
		{
			m_dynamics_world->setGravity(PhysicsUtils::ToBullet(gravity));
		}
	}

	glm::vec3 PhysicsWorld::GetGravity() const
	{
		if (m_dynamics_world)
		{
			return PhysicsUtils::ToGLM(m_dynamics_world->getGravity());
		}
		return glm::vec3(0.0f, -9.81f, 0.0f);
	}

	std::shared_ptr<PhysicsBody> PhysicsWorld::GetBody(size_t index) const
	{
		if (index < m_bodies.size())
			return m_bodies[index];
		return nullptr;
	}

}