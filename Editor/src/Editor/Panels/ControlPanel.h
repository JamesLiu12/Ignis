#pragma once

#include "Editor/Panels/EditorPanel.h"

namespace ignis {

	class EditorSceneLayer;

	class ControlPanel : public EditorPanel
	{
	public:
		ControlPanel(EditorSceneLayer* editor_scene_layer);
		~ControlPanel() override = default;

		void OnImGuiRender() override;
		void OnEvent(EventBase& event) override;

		std::string_view GetName() const override { return "Control Panel"; }
		std::string_view GetID() const override { return "control_panel"; }

	private:
		void RenderPlayStopButtons();
		void RenderCameraSpeedControls();

		EditorSceneLayer* m_editor_scene_layer = nullptr;
		float m_camera_speed = 5.0f;
		float m_camera_speed_multiplier = 1.0f;
	};

} // namespace ignis
