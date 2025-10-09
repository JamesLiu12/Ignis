#include "Camera.h"

namespace ignis
{
	Camera::Camera(float fov_deg, float aspect, float near_clip, float far_clip)
		: Camera(glm::perspective(glm::radians(fov_deg), aspect, near_clip, far_clip)) {
	}

	Camera::Camera(float left, float right, float bottom, float top, float near_clip, float far_clip)
		: Camera(glm::ortho(left, right, bottom, top, near_clip, far_clip)) {
	}

	Camera::Camera(const glm::mat4& projection)
	{
		m_projection = projection;
		m_view = glm::mat4(1.0f);
		m_position = glm::vec3(0.0f, 0.0f, 0.0f);
		m_orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	}

	void Camera::RecalculateViewMatrix()
	{
		const glm::vec3 forward = GetForwardDirection();
		const glm::vec3 up = GetUpDirection();
		m_view = glm::lookAt(m_position, m_position + forward, up);
	}
}