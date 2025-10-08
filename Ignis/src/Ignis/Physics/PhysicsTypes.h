#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <btBulletDynamicsCommon.h>

namespace ignis {

	/**
	 * @brief Type of rigid body in the physics simulation
	 */
	enum class BodyType
	{
		Static,     // Never moves (ground, walls)
		Dynamic,    // Affected by forces and gravity
		Kinematic   // Moves but not affected by physics (moving platforms)
	};

	/**
	 * @brief Type of collision shape
	 */
	enum class ShapeType
	{
		Box,
		Sphere,
		Capsule
	};

	/**
	 * @brief Description for creating a rigid body
	 */
	struct RigidBodyDesc
	{
		BodyType type = BodyType::Dynamic;
		ShapeType shape = ShapeType::Box;
		glm::vec3 position = glm::vec3(0.0f);
		glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		glm::vec3 size = glm::vec3(1.0f);
		float mass = 1.0f;
		float friction = 0.5f;
		float restitution = 0.3f; // Bounciness
	};

	/**
	 * @brief Helper functions to convert between GLM and Bullet types
	 */
	namespace PhysicsUtils
	{
		inline btVector3 ToBullet(const glm::vec3& vec)
		{
			return btVector3(vec.x, vec.y, vec.z);
		}

		inline glm::vec3 ToGLM(const btVector3& vec)
		{
			return glm::vec3(vec.x(), vec.y(), vec.z());
		}

		inline btQuaternion ToBullet(const glm::quat& quat)
		{
			return btQuaternion(quat.x, quat.y, quat.z, quat.w);
		}

		inline glm::quat ToGLM(const btQuaternion& quat)
		{
			return glm::quat(quat.w(), quat.x(), quat.y(), quat.z());
		}
	}

}