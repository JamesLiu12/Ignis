#include "ProjectSerializer.h"
#include "Ignis/Core/File/FileSystem.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using orderer_json = nlohmann::ordered_json;

namespace ignis
{
	bool ProjectSerializer::Serialize(const Project& project, const std::filesystem::path& filepath)
	{
		std::ofstream file(filepath);
		if (!file.is_open())
		{
			return false;
			Log::CoreError("[ProjectSerializer::Serialize] Failed to open file for writing");
		}

		orderer_json data;
		data["ProjectName"] = project.GetProjectName();
		data["ProjectDirectory"] = project.GetProjectDirectory();
		data["AssetDirectory"] = project.GetAssetDirectory();
		data["StartScene"] = project.GetStartScene();

		file << std::setw(4) << data;
	}

	std::shared_ptr<Project> ProjectSerializer::Deserialize(const std::filesystem::path& filepath)
	{
		std::ifstream file(filepath);
		if (!file.is_open())
		{
			return nullptr;
			Log::CoreError("[ProjectSerializer::Serialize] Failed to open file for reading");
		}

		orderer_json data;
		file >> data;

		auto project = std::make_shared<Project>();
		project->m_config.ProjectName = data["ProjectName"];
		project->m_config.AssetDirectory = data["AssetDirectory"];
		project->m_config.ProjectDirectory = data["ProjectDirectory"];
		project->m_config.StartScene = data["StartScene"];

		return project;
	}
}
