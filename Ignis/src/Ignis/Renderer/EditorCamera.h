#pragma once

#include "Camera.h"
#include <glm/glm.hpp>

namespace ignis
{
	// Editor camera with mouse and keyboard controls:
	// - Left mouse drag: Pan
	// - Right mouse drag: Rotate
	// - WASD: Move forward/back/left/right
	// - Q/E: Move down/up
	class EditorCamera : public Camera
	{
	public:
		EditorCamera() = default;
		EditorCamera(float fov_deg, float aspect, float near_clip, float far_clip);
		~EditorCamera() override = default;

		// Update camera based on input (call every frame)
		void OnUpdate(float dt);

		// Camera control settings
		void SetPanSpeed(float speed) { m_pan_speed = speed; }
		void SetRotationSpeed(float speed) { m_rotation_speed = speed; }
		void SetMoveSpeed(float speed) { m_move_speed = speed; }
		
		float GetPanSpeed() const { return m_pan_speed; }
		float GetRotationSpeed() const { return m_rotation_speed; }
		float GetMoveSpeed() const { return m_move_speed; }

		// For debug
		glm::vec3 GetEulerAngles() const { return glm::vec3(m_pitch, m_yaw, 0.0f); }

	private:
		// Camera movement functions
		void Pan(const glm::vec2& delta);
		void Rotate(const glm::vec2& delta);
		void ProcessKeyboardInput(float dt);

		// Update camera orientation from pitch/yaw
		void UpdateCameraOrientation();

	private:
		// Mouse state
		glm::vec2 m_last_mouse_position{ 0.0f, 0.0f };
		bool m_left_mouse_pressed = false;
		bool m_right_mouse_pressed = false;

		// Camera rotation
		float m_pitch = 0.0f;
		float m_yaw = 0.0f;

		// Camera control speeds
		float m_pan_speed = 0.01f;
		float m_rotation_speed = 0.003f;
		float m_move_speed = 5.0f;
	};
}