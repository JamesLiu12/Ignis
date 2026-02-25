#include "Project.h"

namespace ignis
{
	void Project::SetActive(std::shared_ptr<Project> project)
	{
		s_active_project = project;
		VFS::Mount("assets", s_active_project->GetAssetDirectory());
	}
}