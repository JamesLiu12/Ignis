#include "ProjectSerializer.h"
#include "Ignis/Core/File/FileSystem.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;

namespace ignis
{
	bool ProjectSerializer::Serialize(const Project& project, const std::filesystem::path& filepath)
	{
		// Resolve relative paths from executable directory
		std::filesystem::path absolute_filepath;
		if (filepath.is_absolute())
		{
			absolute_filepath = filepath;
		}
		else
		{
			auto exe_dir = FileSystem::GetExecutableDirectory();
			absolute_filepath = exe_dir / filepath;
			absolute_filepath = absolute_filepath.lexically_normal();
		}
		
		std::ofstream file(absolute_filepath);
		if (!file.is_open())
		{
			Log::CoreError("[ProjectSerializer::Serialize] Failed to open file for writing: {}", absolute_filepath.string());
			return false;
		}

		ordered_json data;
		data["ProjectName"] = project.GetProjectName();
		data["AssetDirectory"] = project.GetConfig().AssetDirectory;
		data["AssetRegistry"] = project.GetConfig().AssetRegistry;
		data["StartScene"] = project.GetConfig().StartScene;

		try
		{
			file << data.dump(4);
			file.close();
			Log::CoreInfo("[ProjectSerializer::Serialize] Successfully serialized project to: {}", absolute_filepath.string());
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
		// Resolve relative paths from executable directory
		std::filesystem::path absolute_filepath;
		if (filepath.is_absolute())
		{
			absolute_filepath = filepath;
		}
		else
		{
			auto exe_dir = FileSystem::GetExecutableDirectory();
			absolute_filepath = exe_dir / filepath;
			absolute_filepath = absolute_filepath.lexically_normal();
		}
		
		std::ifstream file(absolute_filepath);
		if (!file.is_open())
		{
			Log::CoreError("[ProjectSerializer::Deserialize] Failed to open file for reading: {}", absolute_filepath.string());
			return nullptr;
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
		project->m_config.ProjectName = data.at("ProjectName").get<std::string>();
		project->m_config.AssetDirectory = data.at("AssetDirectory").get<std::string>();
		project->m_config.AssetRegistry = data.at("AssetRegistry").get<std::string>();
		project->m_config.StartScene = data.at("StartScene").get<std::string>();
		project->m_project_directory = absolute_filepath.parent_path();

		return project;
	}
}
