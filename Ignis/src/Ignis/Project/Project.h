#pragma once

#include "Ignis/Core/File/Path.h"

namespace ignis
{
	class Project
	{
	public:
		Project() = default;
		~Project() = default;

		struct Config
		{
			std::string ProjectName;
			Path AssetDirectory;
			Path AssetRegistry;
			Path StartScene;
		};

		const Config& GetConfig() const { return m_config; }

		const std::string& GetProjectName() const { return m_config.ProjectName; }
		const Path& GetProjectDirectory() const { return m_project_directory; }
		Path GetAssetDirectory() const { return m_project_directory / m_config.AssetDirectory; }
		Path GetAssetRegistry() const { return m_project_directory / m_config.AssetRegistry; }
		Path GetStartScene() const { return GetAssetDirectory() / m_config.StartScene; }

		static const std::string& GetActiveProjectName() { return s_active_project->m_config.ProjectName; }
		static const Path& GetActiveProjectDirectory() { return s_active_project->m_project_directory; }
		static Path GetActiveAssetDirectory() { return s_active_project->m_project_directory / s_active_project->m_config.AssetDirectory; }
		static Path GetActiveAssetRegistry() { return s_active_project->m_project_directory / s_active_project->m_config.AssetRegistry; }
		static Path GetActiveStartScene() { return s_active_project->GetAssetDirectory() / s_active_project->m_config.StartScene; }

		static std::shared_ptr<Project> GetActive() { return s_active_project; }
		static void SetActive(std::shared_ptr<Project> project);

	private:
		Config m_config;
		Path m_project_directory;
		static inline std::shared_ptr<Project> s_active_project;

		friend class ProjectSerializer;
	};
}