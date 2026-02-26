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
		glm::mat4 transform(1.0f);
		Entity entity = *this;

		while (auto parent = entity.GetParent())
		{
			transform *= parent.GetLocalTransform();
		}

		return transform;
	}
}