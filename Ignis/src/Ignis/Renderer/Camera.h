#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace ignis
{
	class Camera
	{
	public:
		Camera() = default;
		Camera(float fov_deg, float aspect, float near_clip, float far_clip);
		Camera(float left, float right, float bottom, float top, float near_clip, float far_clip);
		Camera(const glm::mat4& projection);

		virtual ~Camera() = default;

		void SetPerspective(float fov, float aspect, float near_clip, float far_clip) 
		{ 
			m_projection = glm::perspective(fov, aspect, near_clip, far_clip); 
		}

		void SetOrthographic(float left, float right, float bottom, float top, float near_clip, float far_clip) 
		{ 
			m_projection = glm::ortho(left, right, bottom, top, near_clip , far_clip); 
		}

		void SetProjection(const glm::mat4& projection) { m_projection = projection; }
		void SetView(const glm::mat4& view) { m_view = view; }
		void SetPosition(const glm::vec3& position) { m_position = position; }
		void SetOrientation(const glm::quat& orientation) { m_orientation = orientation; }

		const glm::mat4& GetProjection() const { return m_projection; }
		const glm::mat4& GetView() const { return m_view; }
		const glm::vec3& GetPosition() const { return m_position; }
		
		glm::mat4 GetViewProjection() const { return m_projection * m_view; }
		
		glm::vec3 GetForwardDirection() const
		{
			return glm::mat3_cast(m_orientation) * glm::vec3{ 0.0f, 0.0f, -1.0f };
		}

		glm::vec3 GetUpDirection() const
		{
			return glm::mat3_cast(m_orientation) * glm::vec3{ 0.0f, 1.0f, 0.0f };
		}

		glm::vec3 GetRightDirection() const
		{
			return glm::mat3_cast(m_orientation) * glm::vec3{ 1.0f, 0.0f, 0.0f };
		}

		void RecalculateViewMatrix();

	protected:
		glm::mat4 m_projection{ 1.0f };
		glm::mat4 m_view{ 1.0f };

		glm::vec3 m_position{ 0.0f, 0.0f, 0.0f };
		glm::quat m_orientation{ 1.0f, 0.0f, 0.0f, 0.0f };
	};
}