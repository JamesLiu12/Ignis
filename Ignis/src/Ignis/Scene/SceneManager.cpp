#include "SceneManager.h"
#include "Ignis/Core/Application.h"
#include "Ignis/Core/Log.h"

namespace ignis
{
	// Singleton instance pointer (set by EditorSceneLayer or RuntimeSceneLayer)
	static ISceneLayer* s_scene_layer = nullptr;

	void SceneManager::LoadScene(const std::string& scene_path)
	{
		if (!s_scene_layer)
		{
			Log::CoreError("SceneManager::LoadScene - No scene layer registered");
			return;
		}

		Log::CoreInfo("SceneManager::LoadScene - Queueing scene transition to: {}", scene_path);
		s_scene_layer->QueueSceneTransition(scene_path);
	}

	std::string SceneManager::GetCurrentSceneName()
	{
		if (!s_scene_layer)
		{
			Log::CoreWarn("SceneManager::GetCurrentSceneName - No scene layer registered");
			return "";
		}

		return s_scene_layer->GetCurrentSceneName();
	}

	bool SceneManager::IsLoading()
	{
		if (!s_scene_layer)
		{
			return false;
		}

		return s_scene_layer->HasPendingSceneTransition();
	}

	void SceneManager::RegisterSceneLayer(ISceneLayer* layer)
	{
		s_scene_layer = layer;
		Log::CoreInfo("SceneManager: Scene layer registered");
	}

	void SceneManager::UnregisterSceneLayer()
	{
		s_scene_layer = nullptr;
		// Note: No logging here to avoid crash during shutdown when logger may be destroyed
	}
}
