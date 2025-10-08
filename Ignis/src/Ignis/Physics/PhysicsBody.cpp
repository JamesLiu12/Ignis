#include "PhysicsBody.h"
#include "Ignis/Core/Log.h"

namespace ignis {

	PhysicsBody::PhysicsBody(const RigidBodyDesc& desc, btDiscreteDynamicsWorld* world)
		: m_rigidBody(nullptr)
		, m_shape(nullptr)
		, m_world(world)
		, m_type(desc.type)
		, m_size(desc.size)
	{
		// Create collision shape
		CreateCollisionShape(desc);

		// Calculate local inertia
		btVector3 localInertia(0, 0, 0);
		float mass = (desc.type == BodyType::Dynamic) ? desc.mass : 0.0f;
		if (mass > 0.0f && m_shape)
		{
			m_shape->calculateLocalInertia(mass, localInertia);
		}

		// Create motion state
		btTransform startTransform;
		startTransform.setIdentity();
		startTransform.setOrigin(PhysicsUtils::ToBullet(desc.position));
		startTransform.setRotation(PhysicsUtils::ToBullet(desc.rotation));

		btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);

		// Create rigid body
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, m_shape, localInertia);
		rbInfo.m_friction = desc.friction;
		rbInfo.m_restitution = desc.restitution;

		m_rigidBody = new btRigidBody(rbInfo);

		// Set kinematic flag if needed
		if (desc.type == BodyType::Kinematic)
		{
			m_rigidBody->setCollisionFlags(m_rigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
			m_rigidBody->setActivationState(DISABLE_DEACTIVATION);
		}

		// Add to world
		if (m_world)
		{
			m_world->addRigidBody(m_rigidBody);
		}
	}

	PhysicsBody::~PhysicsBody()
	{
		// Remove from world
		if (m_world && m_rigidBody)
		{
			m_world->removeRigidBody(m_rigidBody);
		}

		// Delete rigid body
		if (m_rigidBody)
		{
			if (m_rigidBody->getMotionState())
			{
				delete m_rigidBody->getMotionState();
			}
			delete m_rigidBody;
		}

		// Delete collision shape
		if (m_shape)
		{
			delete m_shape;
		}
	}

	void PhysicsBody::CreateCollisionShape(const RigidBodyDesc& desc)
	{
		switch (desc.shape)
		{
			case ShapeType::Box:
			{
				btVector3 halfExtents = PhysicsUtils::ToBullet(desc.size * 0.5f);
				m_shape = new btBoxShape(halfExtents);
				break;
			}
			case ShapeType::Sphere:
			{
				float radius = desc.size.x * 0.5f; // Use X component as radius
				m_shape = new btSphereShape(radius);
				break;
			}
			case ShapeType::Capsule:
			{
				float radius = desc.size.x * 0.5f;
				float height = desc.size.y;
				m_shape = new btCapsuleShape(radius, height);
				break;
			}
			default:
				Log::CoreError("Unsupported collision shape type!");
				m_shape = new btBoxShape(btVector3(0.5f, 0.5f, 0.5f));
				break;
		}
	}

	void PhysicsBody::SetPosition(const glm::vec3& position)
	{
		if (!m_rigidBody) return;

		btTransform transform = m_rigidBody->getWorldTransform();
		transform.setOrigin(PhysicsUtils::ToBullet(position));
		m_rigidBody->setWorldTransform(transform);

		if (m_type == BodyType::Dynamic)
		{
			m_rigidBody->activate();
		}
	}

	glm::vec3 PhysicsBody::GetPosition() const
	{
		if (!m_rigidBody) return glm::vec3(0.0f);

		btVector3 origin = m_rigidBody->getWorldTransform().getOrigin();
		return PhysicsUtils::ToGLM(origin);
	}

	void PhysicsBody::SetRotation(const glm::quat& rotation)
	{
		if (!m_rigidBody) return;

		btTransform transform = m_rigidBody->getWorldTransform();
		transform.setRotation(PhysicsUtils::ToBullet(rotation));
		m_rigidBody->setWorldTransform(transform);

		if (m_type == BodyType::Dynamic)
		{
			m_rigidBody->activate();
		}
	}

	glm::quat PhysicsBody::GetRotation() const
	{
		if (!m_rigidBody) return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

		btQuaternion rotation = m_rigidBody->getWorldTransform().getRotation();
		return PhysicsUtils::ToGLM(rotation);
	}

	void PhysicsBody::SetMass(float mass)
	{
		if (!m_rigidBody || !m_shape) return;

		btVector3 localInertia(0, 0, 0);
		if (mass > 0.0f)
		{
			m_shape->calculateLocalInertia(mass, localInertia);
		}

		m_rigidBody->setMassProps(mass, localInertia);
		m_rigidBody->updateInertiaTensor();
	}

	float PhysicsBody::GetMass() const
	{
		if (!m_rigidBody) return 0.0f;

		float invMass = m_rigidBody->getInvMass();
		return (invMass == 0.0f) ? 0.0f : 1.0f / invMass;
	}

	void PhysicsBody::SetLinearVelocity(const glm::vec3& velocity)
	{
		if (!m_rigidBody || m_type != BodyType::Dynamic) return;

		m_rigidBody->setLinearVelocity(PhysicsUtils::ToBullet(velocity));
		m_rigidBody->activate();
	}

	glm::vec3 PhysicsBody::GetLinearVelocity() const
	{
		if (!m_rigidBody) return glm::vec3(0.0f);

		return PhysicsUtils::ToGLM(m_rigidBody->getLinearVelocity());
	}

	void PhysicsBody::SetAngularVelocity(const glm::vec3& velocity)
	{
		if (!m_rigidBody || m_type != BodyType::Dynamic) return;

		m_rigidBody->setAngularVelocity(PhysicsUtils::ToBullet(velocity));
		m_rigidBody->activate();
	}

	glm::vec3 PhysicsBody::GetAngularVelocity() const
	{
		if (!m_rigidBody) return glm::vec3(0.0f);

		return PhysicsUtils::ToGLM(m_rigidBody->getAngularVelocity());
	}

	void PhysicsBody::ApplyForce(const glm::vec3& force, const glm::vec3& relativePos)
	{
		if (!m_rigidBody || m_type != BodyType::Dynamic) return;

		m_rigidBody->applyForce(PhysicsUtils::ToBullet(force), PhysicsUtils::ToBullet(relativePos));
		m_rigidBody->activate();
	}

	void PhysicsBody::ApplyImpulse(const glm::vec3& impulse, const glm::vec3& relativePos)
	{
		if (!m_rigidBody || m_type != BodyType::Dynamic) return;

		m_rigidBody->applyImpulse(PhysicsUtils::ToBullet(impulse), PhysicsUtils::ToBullet(relativePos));
		m_rigidBody->activate();
	}

	void PhysicsBody::ApplyCentralForce(const glm::vec3& force)
	{
		if (!m_rigidBody || m_type != BodyType::Dynamic) return;

		m_rigidBody->applyCentralForce(PhysicsUtils::ToBullet(force));
		m_rigidBody->activate();
	}

	void PhysicsBody::ApplyCentralImpulse(const glm::vec3& impulse)
	{
		if (!m_rigidBody || m_type != BodyType::Dynamic) return;

		m_rigidBody->applyCentralImpulse(PhysicsUtils::ToBullet(impulse));
		m_rigidBody->activate();
	}

	void PhysicsBody::SetFriction(float friction)
	{
		if (!m_rigidBody) return;
		m_rigidBody->setFriction(friction);
	}

	float PhysicsBody::GetFriction() const
	{
		if (!m_rigidBody) return 0.0f;
		return m_rigidBody->getFriction();
	}

	void PhysicsBody::SetRestitution(float restitution)
	{
		if (!m_rigidBody) return;
		m_rigidBody->setRestitution(restitution);
	}

	float PhysicsBody::GetRestitution() const
	{
		if (!m_rigidBody) return 0.0f;
		return m_rigidBody->getRestitution();
	}

}