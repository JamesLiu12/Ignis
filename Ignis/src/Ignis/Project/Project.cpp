#include "Project.h"

namespace ignis
{
	void Project::SetActive(std::shared_ptr<Project> project)
	{
		s_active_project = project;
		
		// Only mount assets if project is not null
		if (s_active_project)
		{
			VFS::Mount("assets", s_active_project->GetAssetDirectory());
		}
	}

	static std::string ReplaceAll(std::string s, const std::string& from, const std::string& to)
	{
		size_t pos = 0;
		while ((pos = s.find(from, pos)) != std::string::npos)
		{
			s.replace(pos, from.length(), to);
			pos += to.length();
		}
		return s;
	}

	static std::string GetPlatformToken()
	{
	#if defined(_WIN32)
			return "Windows";
	#elif defined(__APPLE__)
			return "macOS";
	#elif defined(__linux__)
			return "Linux";
	#else
			return "Unknown";
	#endif
	}

	static std::string GetConfigTokenByMacro()
	{
#if defined(NDEBUG)
		return "Release";
#else
		return "Debug";
#endif
	}

	std::filesystem::path Project::ResolveScriptModulePath() const
	{
		const auto& sm = m_config.ScriptModule;

		const std::string platform = GetPlatformToken();
		const std::string config = GetConfigTokenByMacro();
		const std::string name = sm.Name.empty() ? m_config.ProjectName : sm.Name;

		std::string dir = sm.Directory.string();
		dir = ReplaceAll(dir, "{Platform}", platform);
		dir = ReplaceAll(dir, "{Config}", config);
		dir = ReplaceAll(dir, "{Name}", name);

		std::string filePattern;
	#if defined(_WIN32)
			filePattern = sm.Windows;
	#elif defined(__APPLE__)
			filePattern = sm.macOS;
	#elif defined(__linux__)
			filePattern = sm.Linux;
	#endif
		filePattern = ReplaceAll(filePattern, "{Name}", name);
		filePattern = ReplaceAll(filePattern, "{Platform}", platform);
		filePattern = ReplaceAll(filePattern, "{Config}", config);

		return m_project_directory / dir / filePattern;
	}
}