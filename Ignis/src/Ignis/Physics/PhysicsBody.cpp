#include "PhysicsBody.h"
#include "Ignis/Core/Log.h"

namespace ignis {

	PhysicsBody::PhysicsBody(const RigidBodyDesc& desc, btDiscreteDynamicsWorld* world)
		: m_rigid_body(nullptr)
		, m_shape(nullptr)
		, m_world(world)
		, m_type(desc.type)
		, m_size(desc.size)
	{
		// Create collision shape
		CreateCollisionShape(desc);

		// Calculate local inertia
		btVector3 local_inertia(0, 0, 0);
		float mass = (desc.type == BodyType::Dynamic) ? desc.mass : 0.0f;
		if (mass > 0.0f && m_shape)
		{
			m_shape->calculateLocalInertia(mass, local_inertia);
		}

		// Create motion state
		btTransform start_transform;
		start_transform.setIdentity();
		start_transform.setOrigin(PhysicsUtils::ToBullet(desc.position));
		start_transform.setRotation(PhysicsUtils::ToBullet(desc.rotation));

		btDefaultMotionState* motion_state = new btDefaultMotionState(start_transform);

		// Create rigid body
		btRigidBody::btRigidBodyConstructionInfo rb_info(mass, motion_state, m_shape, local_inertia);
		rb_info.m_friction = desc.friction;
		rb_info.m_restitution = desc.restitution;

		m_rigid_body = new btRigidBody(rb_info);

		// Set kinematic flag if needed
		if (desc.type == BodyType::Kinematic)
		{
			m_rigid_body->setCollisionFlags(m_rigid_body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
			m_rigid_body->setActivationState(DISABLE_DEACTIVATION);
		}

		// Add to world
		if (m_world)
		{
			m_world->addRigidBody(m_rigid_body);
		}
	}

	PhysicsBody::~PhysicsBody()
	{
		// Remove from world
		if (m_world && m_rigid_body)
		{
			m_world->removeRigidBody(m_rigid_body);
		}

		// Delete rigid body
		if (m_rigid_body)
		{
			if (m_rigid_body->getMotionState())
			{
				delete m_rigid_body->getMotionState();
			}
			delete m_rigid_body;
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
				btVector3 half_extents = PhysicsUtils::ToBullet(desc.size * 0.5f);
				m_shape = new btBoxShape(half_extents);
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
		if (!m_rigid_body) return;

		btTransform transform = m_rigid_body->getWorldTransform();
		transform.setOrigin(PhysicsUtils::ToBullet(position));
		m_rigid_body->setWorldTransform(transform);

		if (m_type == BodyType::Dynamic)
		{
			m_rigid_body->activate();
		}
	}

	glm::vec3 PhysicsBody::GetPosition() const
	{
		if (!m_rigid_body) return glm::vec3(0.0f);

		btVector3 origin = m_rigid_body->getWorldTransform().getOrigin();
		return PhysicsUtils::ToGLM(origin);
	}

	void PhysicsBody::SetRotation(const glm::quat& rotation)
	{
		if (!m_rigid_body) return;

		btTransform transform = m_rigid_body->getWorldTransform();
		transform.setRotation(PhysicsUtils::ToBullet(rotation));
		m_rigid_body->setWorldTransform(transform);

		if (m_type == BodyType::Dynamic)
		{
			m_rigid_body->activate();
		}
	}

	glm::quat PhysicsBody::GetRotation() const
	{
		if (!m_rigid_body) return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

		btQuaternion rotation = m_rigid_body->getWorldTransform().getRotation();
		return PhysicsUtils::ToGLM(rotation);
	}

	void PhysicsBody::SetMass(float mass)
	{
		if (!m_rigid_body || !m_shape) return;

		btVector3 local_inertia(0, 0, 0);
		if (mass > 0.0f)
		{
			m_shape->calculateLocalInertia(mass, local_inertia);
		}

		m_rigid_body->setMassProps(mass, local_inertia);
		m_rigid_body->updateInertiaTensor();
	}

	float PhysicsBody::GetMass() const
	{
		if (!m_rigid_body) return 0.0f;

		float inv_mass = m_rigid_body->getInvMass();
		return (inv_mass == 0.0f) ? 0.0f : 1.0f / inv_mass;
	}

	void PhysicsBody::SetLinearVelocity(const glm::vec3& velocity)
	{
		if (!m_rigid_body || m_type != BodyType::Dynamic) return;

		m_rigid_body->setLinearVelocity(PhysicsUtils::ToBullet(velocity));
		m_rigid_body->activate();
	}

	glm::vec3 PhysicsBody::GetLinearVelocity() const
	{
		if (!m_rigid_body) return glm::vec3(0.0f);

		return PhysicsUtils::ToGLM(m_rigid_body->getLinearVelocity());
	}

	void PhysicsBody::SetAngularVelocity(const glm::vec3& velocity)
	{
		if (!m_rigid_body || m_type != BodyType::Dynamic) return;

		m_rigid_body->setAngularVelocity(PhysicsUtils::ToBullet(velocity));
		m_rigid_body->activate();
	}

	glm::vec3 PhysicsBody::GetAngularVelocity() const
	{
		if (!m_rigid_body) return glm::vec3(0.0f);

		return PhysicsUtils::ToGLM(m_rigid_body->getAngularVelocity());
	}

	void PhysicsBody::ApplyForce(const glm::vec3& force, const glm::vec3& relative_pos)
	{
		if (!m_rigid_body || m_type != BodyType::Dynamic) return;

		m_rigid_body->applyForce(PhysicsUtils::ToBullet(force), PhysicsUtils::ToBullet(relative_pos));
		m_rigid_body->activate();
	}

	void PhysicsBody::ApplyImpulse(const glm::vec3& impulse, const glm::vec3& relative_pos)
	{
		if (!m_rigid_body || m_type != BodyType::Dynamic) return;

		m_rigid_body->applyImpulse(PhysicsUtils::ToBullet(impulse), PhysicsUtils::ToBullet(relative_pos));
		m_rigid_body->activate();
	}

	void PhysicsBody::ApplyCentralForce(const glm::vec3& force)
	{
		if (!m_rigid_body || m_type != BodyType::Dynamic) return;

		m_rigid_body->applyCentralForce(PhysicsUtils::ToBullet(force));
		m_rigid_body->activate();
	}

	void PhysicsBody::ApplyCentralImpulse(const glm::vec3& impulse)
	{
		if (!m_rigid_body || m_type != BodyType::Dynamic) return;

		m_rigid_body->applyCentralImpulse(PhysicsUtils::ToBullet(impulse));
		m_rigid_body->activate();
	}

	void PhysicsBody::SetFriction(float friction)
	{
		if (!m_rigid_body) return;
		m_rigid_body->setFriction(friction);
	}

	float PhysicsBody::GetFriction() const
	{
		if (!m_rigid_body) return 0.0f;
		return m_rigid_body->getFriction();
	}

	void PhysicsBody::SetRestitution(float restitution)
	{
		if (!m_rigid_body) return;
		m_rigid_body->setRestitution(restitution);
	}

	float PhysicsBody::GetRestitution() const
	{
		if (!m_rigid_body) return 0.0f;
		return m_rigid_body->getRestitution();
	}

}