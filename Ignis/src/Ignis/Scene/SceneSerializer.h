#pragma once

#include "Ignis/Core/File/Path.h"

namespace ignis
{
	class Scene;

	class SceneSerializer
	{
	public:
		SceneSerializer() = default;

		bool Serialize(const Scene& scene, const Path& filepath);
		std::shared_ptr<Scene> Deserialize(const Path& filepath);
	};
}