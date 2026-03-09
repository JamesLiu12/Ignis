#pragma once

#include "Ignis/Core/API.h"
#include "PhysicsTypes.h"
#include "PhysicsBody.h"
#include <entt.hpp>

class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;

namespace ignis {

	/**
	 * @brief Main physics world that manages the Bullet Physics simulation
	 */
	class IGNIS_API PhysicsWorld
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

		/**
		 * @brief Detect collisions and update collision tracking
		 */
		void DetectCollisions();

		/**
		 * @brief Set entity mapping for collision callbacks
		 */
		void SetEntityMapping(const std::unordered_map<btRigidBody*, entt::entity>& mapping);

		/**
		 * @brief Collision pair structure
		 */
		struct CollisionPair
		{
			entt::entity entity_a;
			entt::entity entity_b;
			
			bool operator==(const CollisionPair& other) const
			{
				return (entity_a == other.entity_a && entity_b == other.entity_b) ||
					   (entity_a == other.entity_b && entity_b == other.entity_a);
			}
		};

		/**
		 * @brief Hash function for CollisionPair
		 */
		struct CollisionPairHash
		{
			size_t operator()(const CollisionPair& pair) const
			{
				size_t h1 = std::hash<uint32_t>{}(static_cast<uint32_t>(pair.entity_a));
				size_t h2 = std::hash<uint32_t>{}(static_cast<uint32_t>(pair.entity_b));
				return h1 ^ (h2 << 1);
			}
		};

		/**
		 * @brief Get active collisions
		 */
		const std::unordered_set<CollisionPair, CollisionPairHash>& GetActiveCollisions() const { return m_active_collisions; }

		/**
		 * @brief Get active triggers
		 */
		const std::unordered_set<CollisionPair, CollisionPairHash>& GetActiveTriggers() const { return m_active_triggers; }

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

		// Collision tracking
		std::unordered_set<CollisionPair, CollisionPairHash> m_active_collisions;
		std::unordered_set<CollisionPair, CollisionPairHash> m_active_triggers;
		std::unordered_map<btRigidBody*, entt::entity> m_body_to_entity;
	};

}