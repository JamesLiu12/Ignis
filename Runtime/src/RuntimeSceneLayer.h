#pragma once

#include "Ignis.h"
#include "Ignis/Core/Layer.h"
#include "Ignis/Renderer/Renderer.h"
#include "Ignis/Scene/Scene.h"
#include "Ignis/Renderer/Pipeline.h"
#include "Ignis/Script/ScriptModule.h"
#include "Ignis/UI/UIRenderer.h"
#include "Ignis/UI/UISystem.h"
#include "Ignis/Scene/SceneManager.h"

namespace ignis {

class RuntimeSceneLayer : public Layer, public ISceneLayer
{
public:
	RuntimeSceneLayer(Renderer& renderer, const std::string& project_path);
	~RuntimeSceneLayer() override;

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(float dt) override;
	void OnEvent(EventBase& event) override;
	
	void LoadScene(const std::filesystem::path& scene_path);
	
	// ISceneLayer interface implementation
	void QueueSceneTransition(const std::filesystem::path& scene_path) override;
	std::string GetCurrentSceneName() const override;
	bool HasPendingSceneTransition() const override;
	
	std::shared_ptr<Scene> GetScene() const { return m_runtime_scene; }

private:
	void ProcessSceneTransition();
	Renderer& m_renderer;
	std::string m_project_path;
	
	std::shared_ptr<Scene> m_runtime_scene;
	std::shared_ptr<Pipeline> m_pipeline;
	ScriptModule m_script_module;
	
	// UI rendering
	UIRenderer m_ui_renderer;
	UISystem m_ui_system;
	
	// Scene transition support
	std::filesystem::path m_pending_scene_path;
	std::filesystem::path m_current_scene_path;
};

} // namespace ignis
