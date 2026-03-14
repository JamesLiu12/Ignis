#pragma once

#include "Ignis/Core/API.h"

namespace ignis
{
	// Forward declare ISceneLayer interface
	class ISceneLayer
	{
	public:
		virtual ~ISceneLayer() = default;
		virtual void QueueSceneTransition(const std::filesystem::path& scene_path) = 0;
		virtual std::string GetCurrentSceneName() const = 0;
		virtual bool HasPendingSceneTransition() const = 0;
	};

	/**
	 * @brief Static API for runtime scene transitions
	 * 
	 * Provides a simple interface for scripts to load scenes during gameplay.
	 * Scene transitions are queued and processed at the end of the frame to
	 * avoid mid-frame loading issues.
	 */
	class IGNIS_API SceneManager
	{
	public:
		// ===== Public API for Scripts =====
		
		/**
		 * @brief Load a scene by path (relative to project root)
		 * @param scene_path Path to the scene file (e.g., "assets/scenes/Level2.igscene")
		 * 
		 * The scene transition is queued and will be processed at the end of the current frame.
		 * This ensures that all scripts and physics have completed their updates before
		 * the scene is changed.
		 */
		static void LoadScene(const std::string& scene_path);
		
		/**
		 * @brief Get the name of the currently active scene
		 * @return Scene name, or empty string if no scene is loaded
		 */
		static std::string GetCurrentSceneName();
		
		/**
		 * @brief Check if a scene transition is currently pending
		 * @return True if a scene is queued to load, false otherwise
		 */
		static bool IsLoading();

		// ===== Internal API (used by EditorSceneLayer/RuntimeSceneLayer) =====
		
		/**
		 * @brief Register a scene layer to handle scene transitions
		 * @param layer Pointer to the scene layer (EditorSceneLayer or RuntimeSceneLayer)
		 * 
		 * This is called internally by scene layers when they are attached.
		 * Only one scene layer can be registered at a time.
		 */
		static void RegisterSceneLayer(ISceneLayer* layer);
		
		/**
		 * @brief Unregister the current scene layer
		 * 
		 * This is called internally by scene layers when they are detached.
		 */
		static void UnregisterSceneLayer();

	private:
		SceneManager() = delete;
		~SceneManager() = delete;
	};
}
