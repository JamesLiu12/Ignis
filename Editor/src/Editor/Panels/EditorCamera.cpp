#include "Editor/Panels/EditorCamera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <imgui.h>

namespace ignis
{
	EditorCamera::EditorCamera(float fov_deg, float aspect, float near_clip, float far_clip)
		: Camera(fov_deg, aspect, near_clip, far_clip)
	{
		// Initialize camera orientation
		m_pitch = 0.0f;
		m_yaw = 0.0f;
	}

	void EditorCamera::OnUpdate(float dt)
	{
		// Process keyboard input first (WASD, Q/E)
		ProcessKeyboardInput(dt);

		// Get current mouse position
		auto [mouse_x, mouse_y] = Input::GetMousePosition();
		glm::vec2 mouse_pos(static_cast<float>(mouse_x), static_cast<float>(mouse_y));
		glm::vec2 delta = mouse_pos - m_last_mouse_position;
		m_last_mouse_position = mouse_pos;

		// Handle left mouse button
		if (Input::IsMouseButtonPressed(MouseButton::Left))
		{
			if (m_left_mouse_pressed)
			{
				Pan(delta);
			}
			m_left_mouse_pressed = true;
		}
		else
		{
			m_left_mouse_pressed = false;
		}

		// Handle right mouse button
		if (Input::IsMouseButtonPressed(MouseButton::Right))
		{
			if (m_right_mouse_pressed)
			{
				Rotate(delta);
			}
			m_right_mouse_pressed = true;
		}
		else
		{
			m_right_mouse_pressed = false;
		}
	}

	void EditorCamera::ProcessKeyboardInput(float dt)
	{
		glm::vec3 forward = GetForwardDirection();
		glm::vec3 right = GetRightDirection();
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

		float velocity = m_move_speed * dt;

		if (Input::IsKeyPressed(KeyCode::W))
		{
			m_position += forward * velocity;
		}
		if (Input::IsKeyPressed(KeyCode::S))
		{
			m_position -= forward * velocity;
		}
		if (Input::IsKeyPressed(KeyCode::A))
		{
			m_position -= right * velocity;
		}
		if (Input::IsKeyPressed(KeyCode::D))
		{
			m_position += right * velocity;
		}

		if (Input::IsKeyPressed(KeyCode::Q))
		{
			m_position -= up * velocity;
		}
		if (Input::IsKeyPressed(KeyCode::E))
		{
			m_position += up * velocity;
		}

		if (Input::IsKeyPressed(KeyCode::W) || Input::IsKeyPressed(KeyCode::S) ||
			Input::IsKeyPressed(KeyCode::A) || Input::IsKeyPressed(KeyCode::D) ||
			Input::IsKeyPressed(KeyCode::Q) || Input::IsKeyPressed(KeyCode::E))
		{
			RecalculateViewMatrix();
		}
	}

	void EditorCamera::Pan(const glm::vec2& delta)
	{
		glm::vec3 right = GetRightDirection();
		glm::vec3 up = GetUpDirection();

		m_position += (-right * delta.x + up * delta.y) * m_pan_speed;

		RecalculateViewMatrix();
	}

	void EditorCamera::Rotate(const glm::vec2& delta)
	{
		m_yaw += delta.x * m_rotation_speed;
		m_pitch -= delta.y * m_rotation_speed;

		const float max_pitch = glm::radians(89.0f);
		m_pitch = glm::clamp(m_pitch, -max_pitch, max_pitch);

		UpdateCameraOrientation();
	}

	void EditorCamera::UpdateCameraOrientation()
	{
		glm::quat pitch_quat = glm::angleAxis(m_pitch, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::quat yaw_quat = glm::angleAxis(m_yaw, glm::vec3(0.0f, 1.0f, 0.0f));
		m_orientation = glm::normalize(yaw_quat * pitch_quat);

		RecalculateViewMatrix();
	}
}