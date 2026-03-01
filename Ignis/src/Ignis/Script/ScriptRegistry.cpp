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
}