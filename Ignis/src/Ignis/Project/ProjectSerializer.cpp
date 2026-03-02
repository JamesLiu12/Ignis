#include "ProjectSerializer.h"
#include "Ignis/Core/File/FileSystem.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;

namespace ignis
{
	bool ProjectSerializer::Serialize(const Project& project, const std::filesystem::path& filepath)
	{
		File file(filepath);
		auto stream = file.OpenOutputStream();
		if (!stream.is_open())
		{
			Log::CoreError("[ProjectSerializer::Serialize] Failed to open file for writing");
			return false;
		}

		ordered_json data;
		data["ProjectName"] = project.GetProjectName();
		data["AssetDirectory"] = FileSystem::ToUnixPath(project.GetConfig().AssetDirectory);
		data["AssetRegistry"] = FileSystem::ToUnixPath(project.GetConfig().AssetRegistry);
		data["StartScene"] = FileSystem::ToUnixPath(project.GetConfig().StartScene);
		data["ScriptModule"]["Name"] = project.GetConfig().ScriptModule.Name;
		data["ScriptModule"]["Directory"] = FileSystem::ToUnixPath(project.GetConfig().ScriptModule.Directory);
		data["ScriptModule"]["Windows"] = project.GetConfig().ScriptModule.Windows;
		data["ScriptModule"]["Linux"] = project.GetConfig().ScriptModule.Linux;
		data["ScriptModule"]["macOS"] = project.GetConfig().ScriptModule.macOS;

		try
		{
			stream << data.dump(4);
			stream.close();
			Log::CoreInfo("[ProjectSerializer::Serialize] Successfully serialized project to: {}", file.GetPath().string());
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
		File file(filepath);
		auto stream = file.OpenInputStream();
		if (!stream.is_open())
		{
			Log::CoreError("[ProjectSerializer::Serialize] Failed to open file for reading");
			return nullptr;
		}

		ordered_json data;
		try
		{
			stream >> data;
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
		project->m_project_directory = file.GetPath().parent_path();

		if (data.contains("ScriptModule"))
		{
			const auto& sm = data["ScriptModule"];
			project->m_config.ScriptModule.Name = sm.value("Name", project->m_config.ProjectName);
			project->m_config.ScriptModule.Directory = sm.value("Directory", "bin/{Platform}/{Config}/");
			project->m_config.ScriptModule.Windows = sm.value("Windows", "{Name}.dll");
			project->m_config.ScriptModule.Linux = sm.value("Linux", "lib{Name}.so");
			project->m_config.ScriptModule.macOS = sm.value("macOS", "lib{Name}.dylib");
		}
		else
		{
			project->m_config.ScriptModule.Name = project->m_config.ProjectName;
			project->m_config.ScriptModule.Directory = "bin/{Platform}/{Config}/";
			project->m_config.ScriptModule.Windows = "{Name}.dll";
			project->m_config.ScriptModule.Linux = "lib{Name}.so";
			project->m_config.ScriptModule.macOS = "lib{Name}.dylib";
		}

		return project;
	}
}
