#include "DebugRenderer.h"
#include "Renderer.h"
#include "Material.h"   // Material::Create / mat->Set / mat->Bind

#include <glm/gtc/constants.hpp>
#include <cmath>

namespace ignis
{
	DebugRenderer::DebugRenderer(Renderer& renderer)
		: m_renderer(renderer)
	{
		m_vertices.reserve(4096);
	}

	void DebugRenderer::Init()
	{
		m_renderer.GetShaderLibrary().Load("resources://shaders/DebugLine.glsl", "DebugLine");
		m_line_shader = m_renderer.GetShaderLibrary().Get("DebugLine");

		// Dynamic VBO updated every Flush()
		m_line_vbo = VertexBuffer::Create(kMaxLineVertices * sizeof(LineVertex),
			VertexBuffer::Usage::Dynamic);
		m_line_vbo->SetLayout({
			{ 0, Shader::DataType::Float3 },   // a_Position
			{ 1, Shader::DataType::Float4 }    // a_Color
			});

		m_line_vao = VertexArray::Create();
		m_line_vao->AddVertexBuffer(m_line_vbo);
	}

	void DebugRenderer::BeginScene(std::shared_ptr<Camera> camera)
	{
		m_camera = camera;
		m_vertices.clear();
	}

	void DebugRenderer::Flush()
	{
		FlushBatch();
	}

	void DebugRenderer::FlushBatch()
	{
		if (m_vertices.empty() || !m_camera || !m_line_shader)
			return;

		m_line_vbo->SetData(m_vertices.data(),
			static_cast<uint32_t>(m_vertices.size() * sizeof(LineVertex)));

		const glm::mat4 view_proj = m_camera->GetProjection() * m_camera->GetView();

		auto mat = Material::Create(m_line_shader);
		mat->Set("u_ViewProjection", view_proj);
		mat->Bind();

		m_renderer.DrawLines(*m_line_vao, static_cast<uint32_t>(m_vertices.size()));

		m_vertices.clear();
	}

	void DebugRenderer::DrawLine(const glm::vec3& p0, const glm::vec3& p1,
		const glm::vec4& color)
	{
		if (m_vertices.size() + 2 > kMaxLineVertices)
			FlushBatch();

		m_vertices.push_back({ p0, color });
		m_vertices.push_back({ p1, color });
	}

	void DebugRenderer::DrawWireBox(const glm::mat4& world_transform,
		const glm::vec3& half_size,
		const glm::vec3& offset,
		const glm::vec4& color)
	{
		// Build 8 corners in local space (offset already baked in)
		const glm::vec3 local[8] = {
			offset + glm::vec3(-half_size.x, -half_size.y, -half_size.z),
			offset + glm::vec3(+half_size.x, -half_size.y, -half_size.z),
			offset + glm::vec3(+half_size.x, +half_size.y, -half_size.z),
			offset + glm::vec3(-half_size.x, +half_size.y, -half_size.z),
			offset + glm::vec3(-half_size.x, -half_size.y, +half_size.z),
			offset + glm::vec3(+half_size.x, -half_size.y, +half_size.z),
			offset + glm::vec3(+half_size.x, +half_size.y, +half_size.z),
			offset + glm::vec3(-half_size.x, +half_size.y, +half_size.z),
		};

		glm::vec3 w[8];
		for (int i = 0; i < 8; ++i)
			w[i] = glm::vec3(world_transform * glm::vec4(local[i], 1.0f));


		DrawLine(w[0], w[1], color);  DrawLine(w[1], w[2], color);
		DrawLine(w[2], w[3], color);  DrawLine(w[3], w[0], color);

		DrawLine(w[4], w[5], color);  DrawLine(w[5], w[6], color);
		DrawLine(w[6], w[7], color);  DrawLine(w[7], w[4], color);

		DrawLine(w[0], w[4], color);  DrawLine(w[1], w[5], color);
		DrawLine(w[2], w[6], color);  DrawLine(w[3], w[7], color);
	}

	void DebugRenderer::DrawWireSphere(const glm::mat4& world_transform,
		float radius,
		const glm::vec3& offset,
		const glm::vec4& color,
		int segments)
	{
		const float step = glm::two_pi<float>() / static_cast<float>(segments);


		auto tp = [&](const glm::vec3& lp) -> glm::vec3
			{
				return glm::vec3(world_transform * glm::vec4(offset + lp, 1.0f));
			};

		for (int i = 0; i < segments; ++i)
		{
			const float a0 = i * step, a1 = (i + 1) * step;
			const float c0 = std::cos(a0), s0 = std::sin(a0);
			const float c1 = std::cos(a1), s1 = std::sin(a1);


			DrawLine(tp({ radius * c0, radius * s0, 0.0f }),
				tp({ radius * c1, radius * s1, 0.0f }), color);

			DrawLine(tp({ radius * c0, 0.0f, radius * s0 }),
				tp({ radius * c1, 0.0f, radius * s1 }), color);

			DrawLine(tp({ 0.0f, radius * c0, radius * s0 }),
				tp({ 0.0f, radius * c1, radius * s1 }), color);
		}
	}

	void DebugRenderer::DrawWireCapsule(const glm::mat4& world_transform,
		float radius,
		float half_height,
		const glm::vec3& offset,
		const glm::vec4& color,
		int segments)
	{
		const float full_step = glm::two_pi<float>() / static_cast<float>(segments);
		const float half_step = glm::pi<float>() / static_cast<float>(segments);

		auto tp = [&](const glm::vec3& lp) -> glm::vec3
			{
				return glm::vec3(world_transform * glm::vec4(offset + lp, 1.0f));
			};


		for (int i = 0; i < segments; ++i)
		{
			const float a0 = i * full_step, a1 = (i + 1) * full_step;
			const float c0 = std::cos(a0), s0 = std::sin(a0);
			const float c1 = std::cos(a1), s1 = std::sin(a1);

			DrawLine(tp({ radius * c0, +half_height, radius * s0 }),
				tp({ radius * c1, +half_height, radius * s1 }), color);

			DrawLine(tp({ radius * c0, -half_height, radius * s0 }),
				tp({ radius * c1, -half_height, radius * s1 }), color);
		}


		for (int i = 0; i < 4; ++i)
		{
			const float angle = i * glm::half_pi<float>();
			const float cx = radius * std::cos(angle);
			const float cz = radius * std::sin(angle);
			DrawLine(tp({ cx, +half_height, cz }),
				tp({ cx, -half_height, cz }), color);
		}


		for (int i = 0; i < segments; ++i)
		{
			const float a0 = i * half_step, a1 = (i + 1) * half_step;
			const float c0 = std::cos(a0), s0 = std::sin(a0);
			const float c1 = std::cos(a1), s1 = std::sin(a1);


			DrawLine(tp({ radius * c0, +half_height + radius * s0, 0.0f }),
				tp({ radius * c1, +half_height + radius * s1, 0.0f }), color);

			DrawLine(tp({ 0.0f, +half_height + radius * s0, radius * c0 }),
				tp({ 0.0f, +half_height + radius * s1, radius * c1 }), color);


			DrawLine(tp({ radius * c0, -half_height - radius * s0, 0.0f }),
				tp({ radius * c1, -half_height - radius * s1, 0.0f }), color);

			DrawLine(tp({ 0.0f, -half_height - radius * s0, radius * c0 }),
				tp({ 0.0f, -half_height - radius * s1, radius * c1 }), color);
		}
	}

	void DebugRenderer::DrawArrow(const glm::vec3& origin,
		const glm::vec3& direction,
		float shaft_length,
		float head_size,
		const glm::vec4& color)
	{
		const glm::vec3 tip = origin + direction * shaft_length;
		DrawLine(origin, tip, color);


		const glm::vec3 perp1 = std::abs(direction.y) < 0.9f
			? glm::normalize(glm::cross(direction, glm::vec3(0.0f, 1.0f, 0.0f)))
			: glm::normalize(glm::cross(direction, glm::vec3(1.0f, 0.0f, 0.0f)));
		const glm::vec3 perp2 = glm::normalize(glm::cross(direction, perp1));

		const glm::vec3 base = tip - direction * head_size;
		DrawLine(tip, base + perp1 * head_size, color);
		DrawLine(tip, base - perp1 * head_size, color);
		DrawLine(tip, base + perp2 * head_size, color);
		DrawLine(tip, base - perp2 * head_size, color);
	}

	void DebugRenderer::DrawCircle(const glm::vec3& center,
		const glm::vec3& normal,
		float radius,
		const glm::vec4& color,
		int segments)
	{

		const glm::vec3 right = std::abs(normal.x) < 0.9f
			? glm::normalize(glm::cross(normal, glm::vec3(1.0f, 0.0f, 0.0f)))
			: glm::normalize(glm::cross(normal, glm::vec3(0.0f, 1.0f, 0.0f)));

		const glm::vec3 up = glm::cross(normal, right);

		const float step = glm::two_pi<float>() / static_cast<float>(segments);
		for (int i = 0; i < segments; ++i)
		{
			const float a0 = i * step, a1 = (i + 1) * step;
			const glm::vec3 p0 = center + (std::cos(a0) * right + std::sin(a0) * up) * radius;
			const glm::vec3 p1 = center + (std::cos(a1) * right + std::sin(a1) * up) * radius;
			DrawLine(p0, p1, color);
		}
	}

	void DebugRenderer::DrawAABB(const glm::vec3& center,
		const glm::vec3& half_size,
		const glm::vec4& color)
	{

		DrawWireBox(glm::mat4(1.0f), half_size, center, color);
	}
}