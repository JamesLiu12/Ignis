#pragma once

namespace ignis
{
	class Scene;

	class SceneSerializer
	{
	public:
		SceneSerializer() = default;

		bool Serialize(const Scene& scene, const std::filesystem::path& filepath);
		std::shared_ptr<Scene> Deserialize(const std::filesystem::path& filepath);
	};
}