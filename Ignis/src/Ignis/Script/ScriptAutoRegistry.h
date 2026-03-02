#pragma once
#include "ScriptRegistry.h"
#include <vector>
#include <string>

namespace ignis
{
	struct AutoScriptEntry
	{
		std::string ClassName;
		ScriptRegistry::FactoryFn Factory;
	};

	inline std::vector<AutoScriptEntry>& GetAutoScriptEntries()
	{
		static std::vector<AutoScriptEntry> entries;
		return entries;
	}

	class AutoScriptRegistrar
	{
	public:
		AutoScriptRegistrar(std::string_view class_name, ScriptRegistry::FactoryFn factory)
		{
			GetAutoScriptEntries().push_back({ std::string(class_name), std::move(factory) });
		}
	};

	inline void RegisterPendingScripts(ScriptRegistry& registry)
	{
		for (auto& e : GetAutoScriptEntries())
			registry.RegisterFactory(e.ClassName, e.Factory);
	}

	inline void UnregisterPendingScripts(ScriptRegistry& registry)
	{
		for (auto& e : GetAutoScriptEntries())
			registry.Unregister(e.ClassName);
	}
}

// ---- macros ----
#define IGNIS_CONCAT_INNER(a, b) a##b
#define IGNIS_CONCAT(a, b) IGNIS_CONCAT_INNER(a, b)

#define IGNIS_SCRIPT_NAMED(TYPE, NAME_STR)                                      \
	namespace {                                                                  \
		::ignis::AutoScriptRegistrar IGNIS_CONCAT(_ignis_auto_reg_, __COUNTER__)( \
			NAME_STR,                                                             \
			[]() -> std::unique_ptr<::ignis::ScriptBehaviour> {                  \
				return std::make_unique<TYPE>();                                 \
			});                                                                   \
	}

#define IGNIS_SCRIPT(TYPE) IGNIS_SCRIPT_NAMED(TYPE, #TYPE)