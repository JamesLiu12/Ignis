#pragma once

#include "Ignis/Core/API.h"
#include "Camera.h"
#include "VertexArray.h"
#include "Shader.h"

#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace ignis
{
	class Renderer;

	class IGNIS_API DebugRenderer
	{
	public:
		explicit DebugRenderer(Renderer& renderer);
		~DebugRenderer() = default;

		void Init();

		void BeginScene(std::shared_ptr<Camera> camera);

		void Flush();

		void DrawLine(const glm::vec3& p0, const glm::vec3& p1,
			const glm::vec4& color = { 0.0f, 1.0f, 0.0f, 1.0f });

		void DrawWireBox(const glm::mat4& world_transform,
			const glm::vec3& half_size,
			const glm::vec3& offset = glm::vec3(0.0f),
			const glm::vec4& color = { 0.0f, 1.0f, 0.0f, 1.0f });

		void DrawWireSphere(const glm::mat4& world_transform,
			float            radius,
			const glm::vec3& offset = glm::vec3(0.0f),
			const glm::vec4& color = { 0.0f, 1.0f, 0.0f, 1.0f },
			int              segments = 32);

		void DrawWireCapsule(const glm::mat4& world_transform,
			float            radius,
			float            half_height,
			const glm::vec3& offset = glm::vec3(0.0f),
			const glm::vec4& color = { 0.0f, 1.0f, 0.0f, 1.0f },
			int              segments = 32);

		void DrawArrow(const glm::vec3& origin,
			const glm::vec3& direction,
			float            shaft_length,
			float            head_size,
			const glm::vec4& color);

		void DrawCircle(const glm::vec3& center,
			const glm::vec3& normal,
			float            radius,
			const glm::vec4& color,
			int              segments = 32);

		void DrawAABB(const glm::vec3& center,
			const glm::vec3& half_size,
			const glm::vec4& color = { 0.0f, 1.0f, 0.0f, 1.0f });

	private:
		void FlushBatch();

		struct LineVertex
		{
			glm::vec3 Position;
			glm::vec4 Color;
		};

		Renderer& m_renderer;
		std::shared_ptr<Camera>       m_camera;

		std::vector<LineVertex>       m_vertices;
		std::shared_ptr<VertexArray>  m_line_vao;
		std::shared_ptr<VertexBuffer> m_line_vbo;
		std::shared_ptr<Shader>       m_line_shader;

		static constexpr uint32_t kMaxLineVertices = 131072u;
	};
}