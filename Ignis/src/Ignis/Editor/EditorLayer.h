#pragma once

#include "Ignis/Core/Layer.h"
#include "PanelManager.h"
#include "EditorCamera.h"
#include "Ignis/Renderer/Mesh.h"
#include "Ignis/Renderer/Pipeline.h"
#include "Ignis/Renderer/ShaderLibrary.h"
#include "Ignis/Scene/Scene.h"
#include "Ignis/Scene/Components.h"
#include "PropertiesPanel.h"
#include "SceneHierarchyPanel.h"

namespace ignis {

	// Main editor layer that manages all editor panels and UI
	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		virtual ~EditorLayer() = default;

		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate(float dt) override;
		void OnImGuiRender() override;
		void OnEvent(EventBase& event) override;

		PanelManager& GetPanelManager() { return *m_panel_manager; }
		
		// Set panel references after they're created by EditorApp
		void SetPropertiesPanel(PropertiesPanel* panel);
		void SetSceneHierarchyPanel(SceneHierarchyPanel* panel);

	private:
		void RenderMenuBar();
		void InitializeScene();

	private:
		std::unique_ptr<PanelManager> m_panel_manager;
		
		// Panel references (owned by EditorApp's PanelManager)
		PropertiesPanel* m_properties_panel = nullptr;
		SceneHierarchyPanel* m_scene_hierarchy_panel = nullptr;
		
		// Scene and rendering
		std::shared_ptr<Scene> m_scene;
		std::shared_ptr<EditorCamera> m_camera;
		std::shared_ptr<ShaderLibrary> m_shader_library;
		std::shared_ptr<Pipeline> m_pipeline;
		
		// Current mesh being edited
		std::shared_ptr<Mesh> m_mesh;
		TransformComponent m_mesh_transform;
	};

} // namespace ignis