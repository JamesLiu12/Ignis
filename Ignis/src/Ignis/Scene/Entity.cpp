#include "Entity.h"

namespace ignis
{
	Entity::Entity(entt::entity handle, entt::registry* registry)
		: m_handle(handle), m_registry(registry)
	{
		
	}
}