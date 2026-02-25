#include "ProjectSerializer.h"
#include "Ignis/Core/File/FileSystem.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;

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

		ordered_json data;
		data["ProjectName"] = project.GetProjectName();
		data["AssetDirectory"] = project.GetAssetDirectory();
		data["AssetRegistry"] = project.GetAssetRegistry();
		data["StartScene"] = project.GetStartScene();

		try
		{
			file << data.dump(4);
			file.close();
			Log::CoreInfo("[ProjectSerializer::Serialize] Successfully serialized project to: {}", filepath.string());
			return true;
		}
		catch (const std::exception& e)
		{
			Log::CoreError("[ProjectSerializer::Serialize] Failed to write JSON: {}", e.what());
			return false;
		}
	}

	std::shared_ptr<Project> ProjectSerializer::Deserialize(const std::filesystem::path& filepath)
	{
		std::ifstream file(filepath);
		if (!file.is_open())
		{
			return nullptr;
			Log::CoreError("[ProjectSerializer::Serialize] Failed to open file for reading");
		}

		ordered_json data;
		try
		{
			file >> data;
		}
		catch (const std::exception& e)
		{
			Log::CoreError("[ProjectSerializer::Deserialize] Failed to parse JSON: {}", e.what());
			return nullptr;
		}

		auto project = std::make_shared<Project>();
		project->m_config.ProjectName = data["ProjectName"];
		project->m_config.AssetDirectory = data["AssetDirectory"];
		project->m_config.AssetRegistry = data["AssetRegistry"];
		project->m_config.StartScene = data["StartScene"];
		project->m_project_directory = filepath.parent_path();

		return project;
	}
}
