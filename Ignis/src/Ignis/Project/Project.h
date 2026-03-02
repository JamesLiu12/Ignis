#pragma once

namespace ignis
{
	class Project
	{
	public:
		Project() = default;
		~Project() = default;

		struct ScriptModuleConfig
		{
			std::string Name;
			std::filesystem::path Directory;
			std::string Windows;
			std::string Linux;
			std::string macOS;
		};

		struct Config
		{
			std::string ProjectName;
			std::filesystem::path AssetDirectory;
			std::filesystem::path AssetRegistry;
			std::filesystem::path StartScene;
			ScriptModuleConfig ScriptModule;
		};

		const Config& GetConfig() const { return m_config; }

		const std::string& GetProjectName() const { return m_config.ProjectName; }
		const std::filesystem::path& GetProjectDirectory() const { return m_project_directory; }
		std::filesystem::path GetAssetDirectory() const { return m_project_directory / m_config.AssetDirectory; }
		std::filesystem::path GetAssetRegistry() const { return m_project_directory / m_config.AssetRegistry; }
		std::filesystem::path GetStartScene() const { return GetAssetDirectory() / m_config.StartScene; }
		const ScriptModuleConfig& GetScriptModuleConfig() const { return m_config.ScriptModule; }
		std::filesystem::path ResolveScriptModulePath() const;

		static const std::string& GetActiveProjectName() { return s_active_project->m_config.ProjectName; }
		static const std::filesystem::path& GetActiveProjectDirectory() { return s_active_project->m_project_directory; }
		static std::filesystem::path GetActiveAssetDirectory() { return s_active_project->m_project_directory / s_active_project->m_config.AssetDirectory; }
		static std::filesystem::path GetActiveAssetRegistry() { return s_active_project->m_project_directory / s_active_project->m_config.AssetRegistry; }
		static std::filesystem::path GetActiveStartScene() { return s_active_project->GetAssetDirectory() / s_active_project->m_config.StartScene; }
		static const ScriptModuleConfig& GetActiveScriptModuleConfig() { return s_active_project->m_config.ScriptModule; }
		static std::filesystem::path ResolveActiveScriptModulePath() { return s_active_project->ResolveScriptModulePath(); }

		static std::shared_ptr<Project> GetActive() { return s_active_project; }
		static void SetActive(std::shared_ptr<Project> project);

	private:
		Config m_config;
		std::filesystem::path m_project_directory;
		static inline std::shared_ptr<Project> s_active_project;

		friend class ProjectSerializer;
	};
}