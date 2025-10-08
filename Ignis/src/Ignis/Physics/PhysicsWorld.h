#pragma once

#include "PhysicsTypes.h"
#include "PhysicsBody.h"
#include <memory>
#include <vector>

class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;

namespace ignis {

	/**
	 * @brief Main physics world that manages the Bullet Physics simulation
	 */
	class PhysicsWorld
	{
	public:
		PhysicsWorld();
		~PhysicsWorld();

		/**
		 * @brief Initialize the physics world
		 */
		void Init();

		/**
		 * @brief Shutdown and cleanup physics resources
		 */
		void Shutdown();

		/**
		 * @brief Step the physics simulation forward
		 * @param delta_time Time step in seconds
		 */
		void Step(float delta_time);

		/**
		 * @brief Create a rigid body in the physics world
		 * @param desc Description of the body to create
		 * @return Shared pointer to the created physics body
		 */
		std::shared_ptr<PhysicsBody> CreateBody(const RigidBodyDesc& desc);

		/**
		 * @brief Remove a body from the physics world
		 * @param body The body to remove
		 */
		void RemoveBody(std::shared_ptr<PhysicsBody> body);

		/**
		 * @brief Set the gravity vector
		 * @param gravity Gravity acceleration (default: 0, -9.81, 0)
		 */
		void SetGravity(const glm::vec3& gravity);

		/**
		 * @brief Get the current gravity vector
		 */
		glm::vec3 GetGravity() const;

		/**
		 * @brief Get the number of bodies in the world
		 */
		size_t GetBodyCount() const { return m_bodies.size(); }

		/**
		 * @brief Get a body by index
		 */
		std::shared_ptr<PhysicsBody> GetBody(size_t index) const;

	private:
		// Bullet Physics components
		btDefaultCollisionConfiguration* m_collision_configuration;
		btCollisionDispatcher* m_dispatcher;
		btBroadphaseInterface* m_broadphase;
		btSequentialImpulseConstraintSolver* m_solver;
		btDiscreteDynamicsWorld* m_dynamics_world;

		// All bodies in the world
		std::vector<std::shared_ptr<PhysicsBody>> m_bodies;

		bool m_initialized;
	};

}