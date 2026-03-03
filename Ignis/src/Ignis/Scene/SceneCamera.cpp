#include "SceneCamera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>

namespace ignis
{
	SceneCamera::SceneCamera()
	{
		RecalculateProjection();
	}

	void SceneCamera::SetProjectionType(ProjectionType type)
	{
		m_projection_type = type;
		RecalculateProjection();
	}

	SceneCamera::ProjectionType SceneCamera::GetProjectionType() const
	{
		return m_projection_type;
	}

	void SceneCamera::SetPerspectiveVerticalFOV(float fov_deg)
	{
		m_perspective_fov_deg = fov_deg;
		RecalculateProjection();
	}

	void SceneCamera::SetPerspectiveNearClip(float near_clip)
	{
		m_perspective_near = near_clip;
		RecalculateProjection();
	}

	void SceneCamera::SetPerspectiveFarClip(float far_clip)
	{
		m_perspective_far = far_clip;
		RecalculateProjection();
	}

	float SceneCamera::GetPerspectiveVerticalFOV() const
	{
		return m_perspective_fov_deg;
	}

	float SceneCamera::GetPerspectiveNearClip() const
	{
		return m_perspective_near;
	}

	float SceneCamera::GetPerspectiveFarClip() const
	{
		return m_perspective_far;
	}

	void SceneCamera::SetOrthographicSize(float size)
	{
		m_orthographic_size = size;
		RecalculateProjection();
	}

	void SceneCamera::SetOrthographicNearClip(float near_clip)
	{
		m_orthographic_near = near_clip;
		RecalculateProjection();
	}

	void SceneCamera::SetOrthographicFarClip(float far_clip)
	{
		m_orthographic_far = far_clip;
		RecalculateProjection();
	}

	float SceneCamera::GetOrthographicSize() const
	{
		return m_orthographic_size;
	}

	float SceneCamera::GetOrthographicNearClip() const
	{
		return m_orthographic_near;
	}

	float SceneCamera::GetOrthographicFarClip() const
	{
		return m_orthographic_far;
	}

	void SceneCamera::SetViewportSize(uint32_t width, uint32_t height)
	{
		if (width == 0 || height == 0)
			return;

		m_aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
		RecalculateProjection();
	}

	void SceneCamera::SetAspectRatio(float aspect_ratio)
	{
		if (aspect_ratio <= 0.0f)
			return;

		m_aspect_ratio = aspect_ratio;
		RecalculateProjection();
	}

	float SceneCamera::GetAspectRatio() const
	{
		return m_aspect_ratio;
	}

	void SceneCamera::RecalculateProjection()
	{
		if (m_projection_type == ProjectionType::Perspective)
		{
			const glm::mat4 proj = glm::perspective(glm::radians(m_perspective_fov_deg), m_aspect_ratio, m_perspective_near, m_perspective_far);
			SetProjection(proj);
		}
		else
		{
			const float half_h = m_orthographic_size * 0.5f;
			const float half_w = half_h * m_aspect_ratio;

			const glm::mat4 proj = glm::ortho(-half_w, half_w, -half_h, half_h, m_orthographic_near, m_orthographic_far);
			SetProjection(proj);
		}
	}
}