#include "Entity.h"
#include "Scene.h"

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


		UUID new_parent_id = new_parent.GetID();
		UUID my_id = GetID();

		auto& my_rel = GetComponent<RelationshipComponent>();
		auto& parent_rel = new_parent.GetComponent<RelationshipComponent>();

		my_rel.ParentID = new_parent_id;

		UUID old_first_child_id = parent_rel.FirstChildID;

		if (old_first_child_id)
		{
			Entity old_first_child = m_scene->GetEntityByID(old_first_child_id);
			auto& old_first_rel = old_first_child.GetComponent<RelationshipComponent>();

			my_rel.NextSiblingID = old_first_child_id;
			old_first_rel.PrevSiblingID = my_id;
		}

		parent_rel.FirstChildID = my_id;
	}

	void Entity::Unparent()
	{
		Entity parent = GetParent();
		if (!parent) return;

		UUID my_id = GetID();

		auto& my_rel = GetComponent<RelationshipComponent>();
		auto& parent_rel = parent.GetComponent<RelationshipComponent>();

		if (my_rel.PrevSiblingID != 0)
		{
			Entity prev_entity = m_scene->GetEntityByID(my_rel.PrevSiblingID);
			prev_entity.GetComponent<RelationshipComponent>().NextSiblingID = my_rel.NextSiblingID;
		}

		if (my_rel.NextSiblingID != 0)
		{
			Entity nextEntity = m_scene->GetEntityByID(my_rel.NextSiblingID);
			nextEntity.GetComponent<RelationshipComponent>().PrevSiblingID = my_rel.PrevSiblingID;
		}

		if (parent_rel.FirstChildID == my_id)
		{
			parent_rel.FirstChildID = my_rel.NextSiblingID;
		}

		my_rel.ParentID = UUID::Invalid;
		my_rel.NextSiblingID = UUID::Invalid;
		my_rel.PrevSiblingID  = UUID::Invalid;
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
}