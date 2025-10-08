#pragma once

#include "PhysicsTypes.h"
#include <memory>

class btRigidBody;
class btCollisionShape;
class btDiscreteDynamicsWorld;

namespace ignis {

	/**
	 * @brief Wrapper for Bullet Physics rigid body
	 */
	class PhysicsBody
	{
	public:
		PhysicsBody(const RigidBodyDesc& desc, btDiscreteDynamicsWorld* world);
		~PhysicsBody();

		// Transform methods
		void SetPosition(const glm::vec3& position);
		glm::vec3 GetPosition() const;
		void SetRotation(const glm::quat& rotation);
		glm::quat GetRotation() const;

		// Physics properties
		void SetMass(float mass);
		float GetMass() const;
		BodyType GetType() const { return m_type; }

		// Velocity methods
		void SetLinearVelocity(const glm::vec3& velocity);
		glm::vec3 GetLinearVelocity() const;
		void SetAngularVelocity(const glm::vec3& velocity);
		glm::vec3 GetAngularVelocity() const;

		// Force methods
		void ApplyForce(const glm::vec3& force, const glm::vec3& relativePos = glm::vec3(0.0f));
		void ApplyImpulse(const glm::vec3& impulse, const glm::vec3& relativePos = glm::vec3(0.0f));
		void ApplyCentralForce(const glm::vec3& force);
		void ApplyCentralImpulse(const glm::vec3& impulse);

		// Material properties
		void SetFriction(float friction);
		float GetFriction() const;
		void SetRestitution(float restitution);
		float GetRestitution() const;

		// Query methods
		bool IsStatic() const { return m_type == BodyType::Static; }
		bool IsDynamic() const { return m_type == BodyType::Dynamic; }
		bool IsKinematic() const { return m_type == BodyType::Kinematic; }

	private:
		btRigidBody* m_rigidBody;
		btCollisionShape* m_shape;
		btDiscreteDynamicsWorld* m_world;
		BodyType m_type;
		glm::vec3 m_size;

		void CreateCollisionShape(const RigidBodyDesc& desc);
	};

}