#pragma once
#include "ScriptBehaviour.h"

namespace ignis
{
	class ScriptRegistry
	{
	public:
		using FactoryFn = std::function<std::unique_ptr<ScriptBehaviour>()>;

		static ScriptRegistry& Get()
		{
			static ScriptRegistry instance;
			return instance;
		}

		template<std::derived_from<ScriptBehaviour> T>
		void Register(std::string_view class_name)
		{
			m_factories[std::string(class_name)] = []() { return std::make_unique<T>(); };
		}

		std::unique_ptr<ScriptBehaviour> Create(std::string_view class_name) const;

		bool Contains(std::string_view class_name) const;

	private:
		std::unordered_map<std::string, FactoryFn> m_factories;
	};
}