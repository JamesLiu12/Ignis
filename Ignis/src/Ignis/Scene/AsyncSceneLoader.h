#pragma once

#include "Ignis/Core/API.h"
#include "Scene.h"
#include <future>

namespace ignis
{
	// Async scene loader for loading scenes in background thread
	// Allows displaying loading screens and prevents frame drops
	class IGNIS_API AsyncSceneLoader
	{
	public:
		AsyncSceneLoader() = default;
		~AsyncSceneLoader();  // Ensure async operations are cleaned up

		// Start loading a scene asynchronously
		// This spawns a background thread to deserialize the scene
		void LoadSceneAsync(const std::filesystem::path& scene_path);

		// Check if the scene has finished loading
		// Returns true when GetScene() is safe to call
		bool IsReady() const;

		// Get the loaded scene (blocks if not ready yet)
		// Only call this after IsReady() returns true
		std::shared_ptr<Scene> GetScene();

		// Get loading progress (0.0 to 1.0)
		// Note: Basic implementation returns 0.0 or 1.0 only
		// Can be enhanced later for granular progress tracking
		float GetProgress() const;

		// Cancel ongoing load operation
		void Cancel();

		// Check if currently loading
		bool IsLoading() const;

	private:
		std::future<std::shared_ptr<Scene>> m_future;
		std::atomic<float> m_progress{ 0.0f };
		std::atomic<bool> m_is_loading{ false };
		std::atomic<bool> m_cancelled{ false };
	};
}
