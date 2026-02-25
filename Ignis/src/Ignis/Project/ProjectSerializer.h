#pragma once

#include "Project.h"

namespace ignis
{
	class ProjectSerializer
	{
	public:
		ProjectSerializer() = default;
		~ProjectSerializer() = default;

		bool Serialize(const Project& project, const Path& filepath);
		std::shared_ptr<Project> Deserialize(const Path& filepath);
	};
}