#pragma once

#include "Ignis/Core/API.h"

#include <filesystem>

namespace ignis
{
	class Scene;

	class IGNIS_API SceneSerializer
	{
	public:
		SceneSerializer() = default;

		bool Serialize(const Scene& scene, const std::filesystem::path& filepath);
		std::shared_ptr<Scene> Deserialize(const std::filesystem::path& filepath);
	};
}