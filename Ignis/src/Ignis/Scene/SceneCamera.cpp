#include "SceneCamera.h"

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

	void SceneCamera::SetAspectRatio(float aspect_ratio)
	{
		m_aspect_ratio = aspect_ratio;
		RecalculateProjection();
	}

	void SceneCamera::SetPerspective(float fov_deg, float near_clip, float far_clip)
	{
		m_projection_type = ProjectionType::Perspective;
		m_perspective_fov = fov_deg;
		m_perspective_near = near_clip;
		m_perspective_far = far_clip;
		RecalculateProjection();
	}

	void SceneCamera::SetPerspectiveFOV(float fov_deg)
	{
		m_perspective_fov = fov_deg;
		if (m_projection_type == ProjectionType::Perspective)
			RecalculateProjection();
	}

	void SceneCamera::SetPerspectiveNearClip(float near_clip)
	{
		m_perspective_near = near_clip;
		if (m_projection_type == ProjectionType::Perspective)
			RecalculateProjection();
	}

	void SceneCamera::SetPerspectiveFarClip(float far_clip)
	{
		m_perspective_far = far_clip;
		if (m_projection_type == ProjectionType::Perspective)
			RecalculateProjection();
	}

	void SceneCamera::SetOrthographic(float size, float near_clip, float far_clip)
	{
		m_projection_type = ProjectionType::Orthographic;
		m_orthographic_size = size;
		m_orthographic_near = near_clip;
		m_orthographic_far = far_clip;
		RecalculateProjection();
	}

	void SceneCamera::SetOrthographicSize(float size)
	{
		m_orthographic_size = size;
		if (m_projection_type == ProjectionType::Orthographic)
			RecalculateProjection();
	}

	void SceneCamera::SetOrthographicNearClip(float near_clip)
	{
		m_orthographic_near = near_clip;
		if (m_projection_type == ProjectionType::Orthographic)
			RecalculateProjection();
	}

	void SceneCamera::SetOrthographicFarClip(float far_clip)
	{
		m_orthographic_far = far_clip;
		if (m_projection_type == ProjectionType::Orthographic)
			RecalculateProjection();
	}

	void SceneCamera::SetViewFromWorldTransform(const glm::mat4& world_transform)
	{
		m_position = glm::vec3(world_transform[3]);

		glm::vec3 col0 = glm::normalize(glm::vec3(world_transform[0]));
		glm::vec3 col1 = glm::normalize(glm::vec3(world_transform[1]));
		glm::vec3 col2 = glm::normalize(glm::vec3(world_transform[2]));
		m_orientation = glm::quat_cast(glm::mat3(col0, col1, col2));

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_position)
			* glm::mat4_cast(m_orientation);
		m_view = glm::inverse(transform);
	}

	void SceneCamera::RecalculateProjection()
	{
		if (m_projection_type == ProjectionType::Perspective)
		{
			m_projection = glm::perspective(
				glm::radians(m_perspective_fov),
				m_aspect_ratio,
				m_perspective_near,
				m_perspective_far
			);
		}
		else
		{
			float half_h = m_orthographic_size * 0.5f;
			float half_w = half_h * m_aspect_ratio;
			m_projection = glm::ortho(
				-half_w, half_w,
				-half_h, half_h,
				m_orthographic_near,
				m_orthographic_far
			);
		}
	}
}