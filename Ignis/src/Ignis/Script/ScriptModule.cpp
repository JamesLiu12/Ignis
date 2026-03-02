#include "ScriptModule.h"

#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace ignis
{
	bool ScriptModule::Load(const std::filesystem::path& module_path)
	{
		Unload();

#if defined(_WIN32)
		HMODULE lib = ::LoadLibraryA(module_path.string().c_str());
		if (!lib)
		{
			Log::CoreError("[ScriptModule] Failed to load module: {}", module_path.string());
			return false;
		}

		m_handle = (void*)lib;
		m_register_fn = (RegisterFn)::GetProcAddress(lib, "RegisterProjectScripts");
		m_unregister_fn = (UnregisterFn)::GetProcAddress(lib, "UnregisterProjectScripts");
#else
		void* lib = dlopen(module_path.string().c_str(), RTLD_NOW);
		if (!lib)
		{
			Log::CoreError("[ScriptModule] Failed to load module: {}", module_path.string());
			return false;
		}

		m_handle = lib;
		m_register_fn = (RegisterFn)dlsym(lib, "RegisterProjectScripts");
		m_unregister_fn = (UnregisterFn)dlsym(lib, "UnregisterProjectScripts");
#endif

		if (!m_register_fn)
		{
			Log::CoreError("[ScriptModule] Missing export: RegisterProjectScripts");
			Unload();
			return false;
		}

		return true;
	}

	bool ScriptModule::RegisterAll(ScriptRegistry& registry)
	{
		if (!m_handle || !m_register_fn)
			return false;

		m_register_fn(registry);
		return true;
	}

	void ScriptModule::UnregisterAll(ScriptRegistry& registry)
	{
		if (!m_handle)
			return;

		if (m_unregister_fn)
			m_unregister_fn(registry);
		else
			registry.Clear();
	}

	void ScriptModule::Unload()
	{
		if (!m_handle)
			return;

#if defined(_WIN32)
		::FreeLibrary((HMODULE)m_handle);
#else
		dlclose(m_handle);
#endif

		m_handle = nullptr;
		m_register_fn = nullptr;
		m_unregister_fn = nullptr;
	}
}