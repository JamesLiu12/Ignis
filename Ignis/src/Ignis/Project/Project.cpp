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
}