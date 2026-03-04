#pragma once

#include "Ignis/Core/API.h"
#include "ScriptRegistry.h"

#include <filesystem>

namespace ignis
{
	class IGNIS_API ScriptModule
	{
	public:
		using RegisterFn = void(*)(ScriptRegistry& registry);
		using UnregisterFn = void(*)(ScriptRegistry& registry);

		bool Load(const std::filesystem::path& module_path);
		void Unload();

		bool IsLoaded() const { return m_handle != nullptr; }

		bool RegisterAll(ScriptRegistry& registry);
		void UnregisterAll(ScriptRegistry& registry);

	private:
		void* m_handle = nullptr;
		RegisterFn m_register_fn = nullptr;
		UnregisterFn m_unregister_fn = nullptr;
	};
}