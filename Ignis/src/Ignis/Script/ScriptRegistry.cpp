#include "ScriptRegistry.h"

namespace ignis
{
	std::unique_ptr<ScriptBehaviour> ScriptRegistry::Create(std::string_view class_name) const
	{
		auto it = m_factories.find(std::string(class_name));
		if (it == m_factories.end())
			return nullptr;
		return it->second();
	}

	bool ScriptRegistry::Contains(std::string_view class_name) const
	{
		return m_factories.contains(std::string(class_name));
	}

	bool ScriptRegistry::RegisterFactory(std::string_view class_name, FactoryFn factory)
	{
		auto key = std::string(class_name);
		m_factories[key] = std::move(factory);
		return true;
	}

	bool ScriptRegistry::Unregister(std::string_view class_name)
	{
		return m_factories.erase(std::string(class_name)) > 0;
	}

	void ScriptRegistry::Clear()
	{
		m_factories.clear();
	}
}