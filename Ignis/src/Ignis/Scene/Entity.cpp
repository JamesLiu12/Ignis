#include "Entity.h"
#include "Scene.h"
#include "Ignis/UI/UIComponents.h"

namespace ignis
{
	Entity::Entity(entt::entity handle, Scene* scene)
		: m_handle(handle), m_scene(scene)
	{
		
	}

	bool Entity::IsValid() const
	{
		return m_handle != entt::null && m_scene && m_scene->m_registry.valid(m_handle);
	}

	Entity::operator bool() const { return IsValid(); }

	UUID Entity::GetID() const
	{
		return GetComponent<IDComponent>().ID;
	}

	UUID Entity::GetParentID() const
	{
		return GetComponent<RelationshipComponent>().ParentID;
	}

	Entity Entity::GetParent() const
	{
		return m_scene->GetEntityByID(GetParentID());
	}

	void Entity::SetParent(Entity new_parent)
	{
		Entity current_parent = GetParent();
		if (current_parent == new_parent) return;

		if (current_parent)
		{
			Unparent();
		}

		if (!new_parent) return;

		// Prevent circular parent references
		Entity ancestor = new_parent;
		int depth = 0;
		constexpr int MAX_DEPTH = 1000;
		
		while (ancestor)
		{
			if (ancestor == *this)
			{
				Log::CoreError("Cannot set parent: would create circular reference for entity {}",
				              GetID().ToString());
				return;
			}
			
			if (++depth > MAX_DEPTH)
			{
				Log::CoreError("Parent chain too deep (>{}) for entity {}, possible corruption",
				              MAX_DEPTH, GetID().ToString());
				return;
			}
			
			ancestor = ancestor.GetParent();
		}

		UUID new_parent_id = new_parent.GetID();
		UUID my_id = GetID();

		auto& my_rel = GetComponent<RelationshipComponent>();
		auto& parent_rel = new_parent.GetComponent<RelationshipComponent>();

		my_rel.ParentID = new_parent_id;

		if (parent_rel.ChildrenCount == 0)
		{
			parent_rel.FirstChildID = my_id;
			parent_rel.LastChildID = my_id;

			my_rel.PrevSiblingID = UUID::Invalid;
			my_rel.NextSiblingID = UUID::Invalid;
		}
		else
		{
			UUID old_last_id = parent_rel.LastChildID;
			Entity old_last_entity = m_scene->GetEntityByID(old_last_id);
			auto& old_last_rel = old_last_entity.GetComponent<RelationshipComponent>();

			old_last_rel.NextSiblingID = my_id;
			my_rel.PrevSiblingID = old_last_id;
			my_rel.NextSiblingID = UUID::Invalid;

			parent_rel.LastChildID = my_id;
		}

		parent_rel.ChildrenCount++;
	}

	void Entity::Unparent()
	{
		Entity parent = GetParent();
		if (!parent) return;

		auto& my_rel = GetComponent<RelationshipComponent>();
		auto& parent_rel = parent.GetComponent<RelationshipComponent>();

		UUID my_id = GetID();
		UUID prev_id = my_rel.PrevSiblingID;
		UUID next_id = my_rel.NextSiblingID;

		if (parent_rel.FirstChildID == my_id)
		{
			parent_rel.FirstChildID = next_id;
		}

		if (parent_rel.LastChildID == my_id)
		{
			parent_rel.LastChildID = prev_id;
		}

		if (prev_id)
		{
			Entity prev_entity = m_scene->GetEntityByID(prev_id);
			prev_entity.GetComponent<RelationshipComponent>().NextSiblingID = next_id;
		}

		if (next_id)
		{
			Entity next_entity = m_scene->GetEntityByID(next_id);
			next_entity.GetComponent<RelationshipComponent>().PrevSiblingID = prev_id;
		}

		my_rel.ParentID = UUID::Invalid;
		my_rel.PrevSiblingID = UUID::Invalid;
		my_rel.NextSiblingID = UUID::Invalid;

		parent_rel.ChildrenCount--;
	}

	void Entity::AddChild(Entity child)
	{
		if (child)
		{
			child.SetParent(*this);
		}
	}

	void Entity::RemoveChild(Entity child)
	{
		if (child && child.GetParent() == *this)
		{
			child.Unparent();
		}
		else
		{
			 Log::Warn("Entity {0} is not a child of {1}", child.GetID().ToString(), GetID().ToString());
		}
	}

	std::vector<Entity> Entity::GetChildren() const
	{
		std::vector<Entity> children;

		const auto& rel = GetComponent<RelationshipComponent>();
		UUID current_id = rel.FirstChildID;

		while (current_id != UUID::Invalid)
		{
			Entity child = m_scene->GetEntityByID(current_id);
			children.push_back(child);

			current_id = child.GetComponent<RelationshipComponent>().NextSiblingID;
		}

		return children;
	}

	void Entity::MoveTo(Entity target_prev_sibling)
	{
		Entity parent = GetParent();

		if (!parent) return;

		if (target_prev_sibling && target_prev_sibling.GetParent() != parent)
		{
			return;
		}

		if (target_prev_sibling == *this) return;

		auto& my_rel = GetComponent<RelationshipComponent>();

		if (target_prev_sibling)
		{
			if (my_rel.PrevSiblingID == target_prev_sibling.GetID()) return;
		}
		else
		{
			if (my_rel.PrevSiblingID == UUID::Invalid) return;
		}

		auto& parent_rel = parent.GetComponent<RelationshipComponent>();

		UUID my_id = GetID();
		UUID old_prev_id = my_rel.PrevSiblingID;
		UUID old_next_id = my_rel.NextSiblingID;

		if (old_prev_id != UUID::Invalid)
		{
			Entity old_prev = m_scene->GetEntityByID(old_prev_id);
			old_prev.GetComponent<RelationshipComponent>().NextSiblingID = old_next_id;
		}
		if (old_next_id != UUID::Invalid)
		{
			Entity old_next = m_scene->GetEntityByID(old_next_id);
			old_next.GetComponent<RelationshipComponent>().PrevSiblingID = old_prev_id;
		}

		if (parent_rel.FirstChildID == my_id)
			parent_rel.FirstChildID = old_next_id;
		if (parent_rel.LastChildID == my_id)
			parent_rel.LastChildID = old_prev_id;


		if (target_prev_sibling)
		{
			UUID target_id = target_prev_sibling.GetID();
			auto& target_rel = target_prev_sibling.GetComponent<RelationshipComponent>();
			UUID target_next_id = target_rel.NextSiblingID;

			my_rel.PrevSiblingID = target_id;
			my_rel.NextSiblingID = target_next_id;

			target_rel.NextSiblingID = my_id;

			if (target_next_id != UUID::Invalid)
			{
				Entity target_next = m_scene->GetEntityByID(target_next_id);
				target_next.GetComponent<RelationshipComponent>().PrevSiblingID = my_id;
			}
			else
			{
				parent_rel.LastChildID = my_id;
			}
		}
		else
		{
			UUID old_first_id = parent_rel.FirstChildID;

			my_rel.PrevSiblingID = UUID::Invalid;
			my_rel.NextSiblingID = old_first_id;

			parent_rel.FirstChildID = my_id;

			if (old_first_id != UUID::Invalid)
			{
				Entity old_first = m_scene->GetEntityByID(old_first_id);
				old_first.GetComponent<RelationshipComponent>().PrevSiblingID = my_id;
			}
			else
			{
				parent_rel.LastChildID = my_id;
			}
		}
	}

	void Entity::SetSiblingIndex(int index)
	{
		Entity parent = GetParent();
		if (!parent) return;

		const auto& parent_rel = parent.GetComponent<RelationshipComponent>();
		int child_count = (int)parent_rel.ChildrenCount;

		if (index < 0) index = 0;
		if (index >= child_count) index = child_count - 1;

		int current_index = -1;
		UUID current_search_id = parent_rel.FirstChildID;
		int search_i = 0;
		while (current_search_id != UUID::Invalid)
		{
			if (current_search_id == GetID())
			{
				current_index = search_i;
				break;
			}
			current_search_id = m_scene->GetEntityByID(current_search_id).GetComponent<RelationshipComponent>().NextSiblingID;
			search_i++;
		}

		if (current_index == -1 || current_index == index) return;

		int target_prev_index;

		if (index < current_index)
		{
			target_prev_index = index - 1;
		}
		else
		{
			target_prev_index = index;
		}

		if (target_prev_index < 0)
		{
			MoveTo(Entity());
		}
		else
		{
			Entity target_prev = Entity();
			UUID curr_id = parent_rel.FirstChildID;
			int curr_idx = 0;

			while (curr_id != UUID::Invalid)
			{
				if (curr_idx == target_prev_index)
				{
					target_prev = m_scene->GetEntityByID(curr_id);
					break;
				}
				curr_id = m_scene->GetEntityByID(curr_id).GetComponent<RelationshipComponent>().NextSiblingID;
				curr_idx++;
			}

			MoveTo(target_prev);
		}
	}

	void Entity::MoveToAfter(Entity target_prev_sibling)
	{
		if (!target_prev_sibling) return;

		Entity target_parent = target_prev_sibling.GetParent();
		Entity current_parent = GetParent();

		if (target_parent != current_parent)
		{
			SetParent(target_parent);
		}

		MoveTo(target_prev_sibling);
	}

	glm::mat4 Entity::GetLocalTransform() const
	{
		return GetComponent<TransformComponent>().GetTransform();
	}

	glm::mat4 Entity::GetWorldTransform() const
	{
		glm::mat4 transform = GetLocalTransform();
		Entity entity = *this;

		// Prevent infinite loops from circular parent references
		constexpr int MAX_HIERARCHY_DEPTH = 1000;
		int depth = 0;

		while (auto parent = entity.GetParent())
		{
			if (++depth > MAX_HIERARCHY_DEPTH)
			{
				Log::CoreError("Entity hierarchy depth exceeded {} - possible circular reference detected for entity {}",
				              MAX_HIERARCHY_DEPTH, GetID().ToString());
				break;
			}

			// Correct transform order: parent * child (not child * parent)
			transform = parent.GetLocalTransform() * transform;
			entity = parent;
		}

		return transform;
	}

	Entity Entity::Duplicate() const
	{
		if (!IsValid())
		{
			Log::CoreWarn("Entity: Cannot duplicate invalid entity");
			return {};
		}

		// Create new entity with duplicated name
		std::string name = GetComponent<TagComponent>().Tag;
		Entity new_entity = m_scene->CreateEntity(name + " (Copy)");

		// Copy all components from this entity to the new entity
		CopyComponentsTo(new_entity);

		// Recursively duplicate children
		for (Entity child : GetChildren())
		{
			Entity child_duplicate = child.Duplicate();
			child_duplicate.SetParent(new_entity);
		}

		Log::CoreInfo("Entity: Duplicated entity '{}' -> '{}'", name, new_entity.GetComponent<TagComponent>().Tag);
		return new_entity;
	}

	void Entity::CopyComponentsTo(Entity destination) const
	{
		// Copy TransformComponent
		if (HasComponent<TransformComponent>())
		{
			auto& src_transform = GetComponent<TransformComponent>();
			auto& dst_transform = destination.GetComponent<TransformComponent>();
			dst_transform.Translation = src_transform.Translation;
			dst_transform.Rotation = src_transform.Rotation;
			dst_transform.Scale = src_transform.Scale;
		}

		// Copy CameraComponent
		if (HasComponent<CameraComponent>())
		{
			auto& src_camera = GetComponent<CameraComponent>();
			auto& dst_camera = destination.AddComponent<CameraComponent>();
			dst_camera.Camera = std::make_shared<SceneCamera>(*src_camera.Camera);
			dst_camera.Primary = src_camera.Primary;
			dst_camera.FixedAspectRatio = src_camera.FixedAspectRatio;
		}

		// Copy DirectionalLightComponent
		if (HasComponent<DirectionalLightComponent>())
		{
			auto& src_light = GetComponent<DirectionalLightComponent>();
			destination.AddComponent<DirectionalLightComponent>(src_light);
		}

		// Copy PointLightComponent
		if (HasComponent<PointLightComponent>())
		{
			auto& src_light = GetComponent<PointLightComponent>();
			destination.AddComponent<PointLightComponent>(src_light);
		}

		// Copy SpotLightComponent
		if (HasComponent<SpotLightComponent>())
		{
			auto& src_light = GetComponent<SpotLightComponent>();
			destination.AddComponent<SpotLightComponent>(src_light);
		}

		// Copy SkyLightComponent
		if (HasComponent<SkyLightComponent>())
		{
			auto& src_light = GetComponent<SkyLightComponent>();
			destination.AddComponent<SkyLightComponent>(src_light);
		}

		// Copy MeshComponent
		if (HasComponent<MeshComponent>())
		{
			auto& src_mesh = GetComponent<MeshComponent>();
			auto& dst_mesh = destination.AddComponent<MeshComponent>();
			dst_mesh.Mesh = src_mesh.Mesh;
			dst_mesh.MaterialSlots = src_mesh.MaterialSlots;
		}

		// Copy ScriptComponent
		if (HasComponent<ScriptComponent>())
		{
			auto& src_script = GetComponent<ScriptComponent>();
			auto& dst_script = destination.AddComponent<ScriptComponent>();
			dst_script.ClassName = src_script.ClassName;
			dst_script.Enabled = src_script.Enabled;
		}

		// Copy TextComponent
		if (HasComponent<TextComponent>())
		{
			auto& src_text = GetComponent<TextComponent>();
			destination.AddComponent<TextComponent>(src_text);
		}

		// Copy AudioSourceComponent
		if (HasComponent<AudioSourceComponent>())
		{
			auto& src_audio = GetComponent<AudioSourceComponent>();
			destination.AddComponent<AudioSourceComponent>(src_audio);
		}

		// Copy AudioListenerComponent
		if (HasComponent<AudioListenerComponent>())
		{
			auto& src_listener = GetComponent<AudioListenerComponent>();
			destination.AddComponent<AudioListenerComponent>(src_listener);
		}

		// Copy RigidBodyComponent (create new physics body, don't share)
		if (HasComponent<RigidBodyComponent>())
		{
			auto& src_rb = GetComponent<RigidBodyComponent>();
			auto& dst_rb = destination.AddComponent<RigidBodyComponent>();
			dst_rb.BodyType = src_rb.BodyType;
			dst_rb.Mass = src_rb.Mass;
			dst_rb.LinearDrag = src_rb.LinearDrag;
			dst_rb.AngularDrag = src_rb.AngularDrag;
			dst_rb.UseGravity = src_rb.UseGravity;
			dst_rb.IsKinematic = src_rb.IsKinematic;
			dst_rb.LockPositionX = src_rb.LockPositionX;
			dst_rb.LockPositionY = src_rb.LockPositionY;
			dst_rb.LockPositionZ = src_rb.LockPositionZ;
			dst_rb.LockRotationX = src_rb.LockRotationX;
			dst_rb.LockRotationY = src_rb.LockRotationY;
			dst_rb.LockRotationZ = src_rb.LockRotationZ;
			// RuntimeBody will be created when physics system initializes
		}

		// Copy BoxColliderComponent
		if (HasComponent<BoxColliderComponent>())
		{
			auto& src_collider = GetComponent<BoxColliderComponent>();
			destination.AddComponent<BoxColliderComponent>(src_collider);
		}

		// Copy SphereColliderComponent
		if (HasComponent<SphereColliderComponent>())
		{
			auto& src_collider = GetComponent<SphereColliderComponent>();
			destination.AddComponent<SphereColliderComponent>(src_collider);
		}

		// Copy CapsuleColliderComponent
		if (HasComponent<CapsuleColliderComponent>())
		{
			auto& src_collider = GetComponent<CapsuleColliderComponent>();
			destination.AddComponent<CapsuleColliderComponent>(src_collider);
		}
	}
}