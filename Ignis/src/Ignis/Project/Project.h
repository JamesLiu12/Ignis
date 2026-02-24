#pragma once

namespace ignis
{
	class Project
	{
	public:
		Project() = default;
		~Project() = default;

		class Config
		{
			std::string ProjectName;
			std::string ProjectDirectory;
			std::string AssetDirectory;
			std::string StartScene;
		};

		const Config& GetConfig() const { return m_config; }

		const std::string& GetProjectName() { return m_config.ProjectName; }
		const std::string& GetProjectDirectory() { return m_config.ProjectDirectory; }
		const std::string& GetAssetDirectory() { return m_config.AssetDirectory; }
		const std::string& GetStartScene() { return m_config.StartScene; }

		static const std::string& GetProjectName() { return s_active_project->m_config.ProjectName; }
		static const std::string& GetProjectDirectory() { return s_active_project->m_config.ProjectDirectory; }
		static const std::string& GetAssetDirectory() { return s_active_project->m_config.AssetDirectory; }
		static const std::string& GetStartScene() { return s_active_project->m_config.StartScene; }

		static std::shared_ptr<Project> GetActive() const { return s_active_project; }
		static void SetActive(std::shared_ptr<Project> project) { s_active_project = project; }

	private:
		Config m_config;

		static inline std::shared_ptr<Project> s_active_project;
	};
}