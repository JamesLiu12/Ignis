#include "AsyncSceneLoader.h"
#include "SceneSerializer.h"
#include "Ignis/Core/Log.h"

namespace ignis
{
	AsyncSceneLoader::~AsyncSceneLoader()
	{
		// Cancel any ongoing operation
		m_cancelled = true;
		
		// Wait for async operation to complete before destroying
		if (m_future.valid())
		{
			// Don't log here - logger may be destroyed during shutdown
			m_future.wait();
			try {
				m_future.get(); // Clear the future
			} catch (...) {
				// Silently catch any exceptions during shutdown
			}
		}
	}

	void AsyncSceneLoader::LoadSceneAsync(const std::filesystem::path& scene_path)
	{
		// Cancel any existing load operation
		if (m_is_loading)
		{
			Log::CoreWarn("AsyncSceneLoader: Cancelling previous load operation");
			Cancel();
		}

		// Reset state
		m_progress = 0.0f;
		m_cancelled = false;
		m_is_loading = true;

		// Launch async task
		m_future = std::async(std::launch::async, [this, scene_path]() -> std::shared_ptr<Scene>
		{
			Log::CoreInfo("AsyncSceneLoader: Starting async load of '{}'", scene_path.string());

			// Check if cancelled before starting
			if (m_cancelled)
			{
				Log::CoreWarn("AsyncSceneLoader: Load cancelled before starting");
				m_is_loading = false;
				return nullptr;
			}

			// Deserialize the scene
			SceneSerializer serializer;
			auto scene = serializer.Deserialize(scene_path);

			// Check if cancelled after deserialization
			if (m_cancelled)
			{
				Log::CoreWarn("AsyncSceneLoader: Load cancelled after deserialization");
				m_is_loading = false;
				return nullptr;
			}

			if (!scene)
			{
				Log::CoreError("AsyncSceneLoader: Failed to load scene from '{}'", scene_path.string());
				m_progress = 0.0f;
				m_is_loading = false;
				return nullptr;
			}

			// Mark as complete
			m_progress = 1.0f;
			m_is_loading = false;
			Log::CoreInfo("AsyncSceneLoader: Successfully loaded '{}'", scene_path.string());

			return scene;
		});
	}

	bool AsyncSceneLoader::IsReady() const
	{
		if (!m_future.valid())
			return false;

		// Check if future is ready without blocking
		return m_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
	}

	std::shared_ptr<Scene> AsyncSceneLoader::GetScene()
	{
		if (!m_future.valid())
		{
			Log::CoreError("AsyncSceneLoader: No scene is being loaded");
			return nullptr;
		}

		// This will block until the scene is ready
		auto scene = m_future.get();
		m_is_loading = false;

		return scene;
	}

	float AsyncSceneLoader::GetProgress() const
	{
		return m_progress.load();
	}

	void AsyncSceneLoader::Cancel()
	{
		m_cancelled = true;

		// Wait for the async operation to finish
		if (m_future.valid())
		{
			m_future.wait();
			m_future.get(); // Clear the future
		}

		m_is_loading = false;
		m_progress = 0.0f;
	}

	bool AsyncSceneLoader::IsLoading() const
	{
		return m_is_loading.load();
	}
}
