#pragma once

#include "Ignis/Renderer/Camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace ignis
{
	class SceneCamera : public Camera
	{
	public:
		enum class ProjectionType { Perspective = 0, Orthographic = 1 };

		SceneCamera();
		~SceneCamera() override = default;

		void SetProjectionType(ProjectionType type);
		ProjectionType GetProjectionType() const { return m_projection_type; }

		void SetAspectRatio(float aspect_ratio);
		float GetAspectRatio() const { return m_aspect_ratio; }

		void SetPerspective(float fov_deg, float near_clip, float far_clip);

		float GetPerspectiveFOV()       const { return m_perspective_fov; }
		float GetPerspectiveNearClip()  const { return m_perspective_near; }
		float GetPerspectiveFarClip()   const { return m_perspective_far; }

		void SetPerspectiveFOV(float fov_deg);
		void SetPerspectiveNearClip(float near_clip);
		void SetPerspectiveFarClip(float far_clip);

		void SetOrthographic(float size, float near_clip, float far_clip);

		float GetOrthographicSize()     const { return m_orthographic_size; }
		float GetOrthographicNearClip() const { return m_orthographic_near; }
		float GetOrthographicFarClip()  const { return m_orthographic_far; }

		void SetOrthographicSize(float size);
		void SetOrthographicNearClip(float near_clip);
		void SetOrthographicFarClip(float far_clip);

		void SetViewFromWorldTransform(const glm::mat4& world_transform);

	private:
		void RecalculateProjection();

	private:
		ProjectionType m_projection_type = ProjectionType::Perspective;

		float m_perspective_fov = 45.0f;
		float m_perspective_near = 0.01f;
		float m_perspective_far = 1000.0f;

		float m_orthographic_size = 10.0f;
		float m_orthographic_near = -1.0f;
		float m_orthographic_far = 1.0f;

		float m_aspect_ratio = 16.0f / 9.0f;
	};
}