#include "PhysicsWorld.h"
#include "Ignis/Core/Log.h"

namespace ignis {

	PhysicsWorld::PhysicsWorld()
		: m_collisionConfiguration(nullptr)
		, m_dispatcher(nullptr)
		, m_broadphase(nullptr)
		, m_solver(nullptr)
		, m_dynamicsWorld(nullptr)
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
		m_collisionConfiguration = new btDefaultCollisionConfiguration();

		// Step 2: Create collision dispatcher
		m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);

		// Step 3: Create broadphase (spatial optimization)
		m_broadphase = new btDbvtBroadphase();

		// Step 4: Create constraint solver
		m_solver = new btSequentialImpulseConstraintSolver();

		// Step 5: Create dynamics world
		m_dynamicsWorld = new btDiscreteDynamicsWorld(
			m_dispatcher,
			m_broadphase,
			m_solver,
			m_collisionConfiguration
		);

		// Step 6: Set default gravity
		m_dynamicsWorld->setGravity(btVector3(0.0f, -9.81f, 0.0f));

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
		if (m_dynamicsWorld)
		{
			// Remove all remaining collision objects
			for (int i = m_dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
			{
				btCollisionObject* obj = m_dynamicsWorld->getCollisionObjectArray()[i];
				btRigidBody* body = btRigidBody::upcast(obj);
				if (body && body->getMotionState())
				{
					delete body->getMotionState();
				}
				m_dynamicsWorld->removeCollisionObject(obj);
				delete obj;
			}

			delete m_dynamicsWorld;
			m_dynamicsWorld = nullptr;
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
		delete m_collisionConfiguration;
		m_collisionConfiguration = nullptr;

		m_initialized = false;
		Log::CoreInfo("PhysicsWorld shutdown complete");
	}

	void PhysicsWorld::Step(float deltaTime)
	{
		if (!m_initialized || !m_dynamicsWorld)
			return;

		// Step the simulation
		// Parameters: timeStep, maxSubSteps
		m_dynamicsWorld->stepSimulation(deltaTime, 10);
	}

	std::shared_ptr<PhysicsBody> PhysicsWorld::CreateBody(const RigidBodyDesc& desc)
	{
		if (!m_initialized || !m_dynamicsWorld)
		{
			Log::CoreError("Cannot create body: PhysicsWorld not initialized!");
			return nullptr;
		}

		auto body = std::make_shared<PhysicsBody>(desc, m_dynamicsWorld);
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
		if (m_dynamicsWorld)
		{
			m_dynamicsWorld->setGravity(PhysicsUtils::ToBullet(gravity));
		}
	}

	glm::vec3 PhysicsWorld::GetGravity() const
	{
		if (m_dynamicsWorld)
		{
			return PhysicsUtils::ToGLM(m_dynamicsWorld->getGravity());
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