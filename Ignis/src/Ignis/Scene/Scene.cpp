#include "Scene.h"
#include "Entity.h"

namespace ignis
{
	Entity Scene::CreateEntity(const std::string name)
	{
		Entity entity = Entity(m_registry.create(), &m_registry);
		entity.AddComponent<TransformComponent>();
		entity.AddComponent<TagComponent>(name.empty() ? "Entity" : name);
		return entity;
	}
}