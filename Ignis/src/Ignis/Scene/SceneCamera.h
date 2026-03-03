#pragma once

#include "Ignis/Renderer/Camera.h"

#include <cstdint>

namespace ignis
{
	class SceneCamera : public Camera
	{
	public:
		enum class ProjectionType : uint8_t
		{
			Perspective = 0,
			Orthographic = 1
		};

	public:
		SceneCamera();
		~SceneCamera() override = default;

		void SetProjectionType(ProjectionType type);
		ProjectionType GetProjectionType() const;

		// Perspective
		void SetPerspectiveVerticalFOV(float fov_deg);
		void SetPerspectiveNearClip(float near_clip);
		void SetPerspectiveFarClip(float far_clip);

		float GetPerspectiveVerticalFOV() const;
		float GetPerspectiveNearClip() const;
		float GetPerspectiveFarClip() const;

		// Orthographic
		void SetOrthographicSize(float size);
		void SetOrthographicNearClip(float near_clip);
		void SetOrthographicFarClip(float far_clip);

		float GetOrthographicSize() const;
		float GetOrthographicNearClip() const;
		float GetOrthographicFarClip() const;

		// Viewport
		void SetViewportSize(uint32_t width, uint32_t height);
		void SetAspectRatio(float aspect_ratio);
		float GetAspectRatio() const;

		void RecalculateProjection();

	private:
		ProjectionType m_projection_type = ProjectionType::Perspective;

		float m_perspective_fov_deg = 45.0f;
		float m_perspective_near = 0.01f;
		float m_perspective_far = 1000.0f;

		float m_orthographic_size = 10.0f;
		float m_orthographic_near = -1.0f;
		float m_orthographic_far = 1.0f;

		float m_aspect_ratio = 16.0f / 9.0f;
	};
}