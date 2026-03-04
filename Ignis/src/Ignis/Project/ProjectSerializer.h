#pragma once

#include "Ignis/Core/API.h"
#include "Project.h"

namespace ignis
{
	class IGNIS_API ProjectSerializer
	{
	public:
		ProjectSerializer() = default;
		~ProjectSerializer() = default;

		bool Serialize(const Project& project, const std::filesystem::path& filepath);
		std::shared_ptr<Project> Deserialize(const std::filesystem::path& filepath);
	};
}