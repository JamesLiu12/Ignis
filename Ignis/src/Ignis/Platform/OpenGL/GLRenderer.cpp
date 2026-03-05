#include "GLRenderer.h"
#include "Ignis/Renderer/Shader.h"

#include <glad/glad.h>

namespace ignis
{
	namespace
	{
		static constexpr float kCubeVertices[] = {
			-1.0f,  1.0f, -1.0f,  -1.0f, -1.0f, -1.0f,   1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,   1.0f,  1.0f, -1.0f,  -1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,  -1.0f, -1.0f, -1.0f,  -1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,  -1.0f,  1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,

			 1.0f, -1.0f, -1.0f,   1.0f, -1.0f,  1.0f,   1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,   1.0f,  1.0f, -1.0f,   1.0f, -1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,   1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,   1.0f, -1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,

			-1.0f,  1.0f, -1.0f,   1.0f,  1.0f, -1.0f,   1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,  -1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f, -1.0f,  -1.0f, -1.0f,  1.0f,   1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,  -1.0f, -1.0f,  1.0f,   1.0f, -1.0f,  1.0f
		};

		static constexpr float kQuadVertices[] = {
			// pos      // uv
			-1.0f,  1.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f,
			 1.0f, -1.0f, 1.0f, 0.0f,

			-1.0f,  1.0f, 0.0f, 1.0f,
			 1.0f, -1.0f, 1.0f, 0.0f,
			 1.0f,  1.0f, 1.0f, 1.0f
		};

		static constexpr uint32_t kMaxTextQuads = 4096;
		static constexpr uint32_t kMaxTextVertices = kMaxTextQuads * 4;
	}

	GLRenderer::GLRenderer()
	{

	}

	void GLRenderer::Init()
	{
		m_shader_library = std::make_unique<ShaderLibrary>();

		m_shader_library->Load("resources://shaders/IgnisPBR.glsl", "IgnisPBR");
		m_shader_library->Load("resources://shaders/Skybox.glsl", "Skybox");
		m_shader_library->Load("resources://shaders/EquirectToCube.glsl", "EquirectToCube");
		m_shader_library->Load("resources://shaders/IrradianceConvolution.glsl", "IrradianceConvolution");
		m_shader_library->Load("resources://shaders/PrefilterGGX.glsl", "PrefilterGGX");
		m_shader_library->Load("resources://shaders/BRDFIntegration.glsl", "BRDFIntegration");
		m_shader_library->Load("resources://shaders/Text.glsl", "Text");

		// Cube
		m_cube_vao = VertexArray::Create();
		auto cube_vbo = VertexBuffer::Create(kCubeVertices, sizeof(kCubeVertices));
		cube_vbo->SetLayout({ { 0, Shader::DataType::Float3 } });
		m_cube_vao->AddVertexBuffer(cube_vbo);

		// Quad
		m_quad_vao = VertexArray::Create();
		auto quad_vbo = VertexBuffer::Create(kQuadVertices, sizeof(kQuadVertices));
		quad_vbo->SetLayout({
			{ 0, Shader::DataType::Float2 },
			{ 1, Shader::DataType::Float2 }
			});
		m_quad_vao->AddVertexBuffer(quad_vbo);

		m_shader_library->Load("resources://screen.glsl");

		// Text
		m_text_vbo = VertexBuffer::Create(kMaxTextVertices * sizeof(float) * 4, VertexBuffer::Usage::Dynamic);
		m_text_vbo->SetLayout({
			{ 0, Shader::DataType::Float2 },   // a_Position
			{ 1, Shader::DataType::Float2 }    // a_TexCoord
		});

		m_text_vao = VertexArray::Create();
		m_text_vao->AddVertexBuffer(m_text_vbo);
	}

	void GLRenderer::BeginFrame()
	{
		if (m_framebuffer)
			m_framebuffer->Bind();

		glEnable(GL_DEPTH_TEST);
	}

	void GLRenderer::EndFrame()
	{
		if (m_framebuffer)
		{
			m_framebuffer->UnBind();
			// ImGui expects a clean buffer to render correctly
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}
	}

	void GLRenderer::SetClearColor(float r, float g, float b, float a)
	{
		glClearColor(r, g, b, a);
	}

	void GLRenderer::SetClearColor(const glm::vec4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void GLRenderer::SetViewport(int x, int y, int width, int height)
	{
		glViewport(x, y, width, height);
	}

	void GLRenderer::SetViewport(const glm::ivec4& viewport)
	{
		glViewport(viewport.x, viewport.y, viewport.z, viewport.w);
	}

	void GLRenderer::DrawIndexed(VertexArray& va)
	{
		va.Bind();
		glDrawElements(GL_TRIANGLES, va.GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
		va.UnBind();
	}

	void GLRenderer::RenderMesh(const Mesh& mesh, const glm::mat4& model, 
		const Environment& scene_environment, const EnvironmentSettings& environment_settings, const LightEnvironment& light_environment)
	{
		auto vao = mesh.GetVertexArray();
		vao->Bind();

		const auto& materials_data = mesh.GetMaterialsData();

		for (const auto& sm : mesh.GetSubmeshes())
		{
			const auto& material_data = materials_data[sm.MaterialIndex];

			// TODO a performance bottleneck?
			auto material = m_pipeline->CreateMaterial(material_data);
			m_pipeline->ApplyEnvironment(*material, scene_environment, environment_settings, light_environment);

			material->Set("view", m_camera->GetView());
			material->Set("projection", m_camera->GetProjection());

			// TODO hard coded to be refactored
			material->Set("model", model);
			material->Set("viewPos", m_camera->GetPosition());

			material->Bind();
			glDrawElements(
				GL_TRIANGLES,
				sm.IndexCount,
				GL_UNSIGNED_INT,
				(void*)(sm.BaseIndex * sizeof(uint32_t))
			);
		}

		vao->UnBind();
	}

	void GLRenderer::RenderSkybox(const Environment& environment)
	{
		auto material = m_pipeline->CreateSkyboxMaterial(environment);

		glDepthFunc(GL_LEQUAL);
		glDisable(GL_CULL_FACE);

		glm::mat4 view = glm::mat4(glm::mat3(m_camera->GetView()));
		material->Set("view", view);
		material->Set("projection", m_camera->GetProjection());

		material->Bind();
		RenderCube();

		glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);
	}

	void GLRenderer::RenderText(const Font& font, const std::string& text, const glm::mat4& transform, const glm::vec4& color, float scale)
	{
		if (text.empty() || !font.GetAtlas())
			return;

		struct Vertex { float x, y, u, v; };
		std::vector<Vertex>   vertices;
		std::vector<uint32_t> indices;
		vertices.reserve(text.size() * 4);
		indices.reserve(text.size() * 6);

		float    cursor_x = 0.0f;
		float    cursor_y = 0.0f;
		uint32_t quad_idx = 0;

		for (unsigned char c : text)
		{
			if (c == '\n')
			{
				cursor_x = 0.0f;
				cursor_y += font.GetLineHeight() * scale;
				continue;
			}

			const GlyphMetrics* g = font.GetGlyph(static_cast<char>(c));
			if (!g)
			{
				if (const GlyphMetrics* sp = font.GetGlyph(' '))
					cursor_x += sp->Advance * scale;
				continue;
			}

			const float px0 = cursor_x + g->QuadMin.x * scale;
			const float py0 = cursor_y - g->QuadMax.y * scale;
			const float px1 = cursor_x + g->QuadMax.x * scale;
			const float py1 = cursor_y - g->QuadMin.y * scale;

			const float u0 = g->AtlasMin.x, v0 = g->AtlasMin.y;
			const float u1 = g->AtlasMax.x, v1 = g->AtlasMax.y;

			vertices.push_back({ px0, py1, u0, v0 });
			vertices.push_back({ px1, py1, u1, v0 });
			vertices.push_back({ px1, py0, u1, v1 });
			vertices.push_back({ px0, py0, u0, v1 });

			const uint32_t b = quad_idx * 4;
			indices.insert(indices.end(), { b, b + 1, b + 2,  b, b + 2, b + 3 });

			cursor_x += g->Advance * scale;
			++quad_idx;
		}

		if (vertices.empty())
			return;

		m_text_vbo->SetData(vertices.data(), vertices.size() * sizeof(Vertex));

		auto ibo = IndexBuffer::Create(
			indices.data(),
			static_cast<uint32_t>(indices.size() * sizeof(uint32_t)));
		m_text_vao->SetIndexBuffer(ibo);

		// TODO: Get Screen Size
		const glm::vec2 screen_size = { 1920.0f, 1080.0f };

		auto mat = Material::Create(m_shader_library->Get("Text"));
		mat->Set("u_Model", transform);
		mat->Set("u_View", m_camera->GetView());
		mat->Set("u_Projection", m_camera->GetProjection());
		mat->Set("u_ScreenSize", screen_size);
		mat->Set("u_Color", color);
		mat->Set("u_Atlas", font.GetAtlas());
		font.GetAtlas()->Bind(0);

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask(GL_FALSE);

		mat->Bind();
		DrawIndexed(*m_text_vao);

		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
		glEnable(GL_CULL_FACE);
	}

	void GLRenderer::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void GLRenderer::SetFramebuffer(std::shared_ptr<Framebuffer> framebuffer)
	{
		m_framebuffer = framebuffer;
	}

	void GLRenderer::RenderCube()
	{
		m_cube_vao->Bind();
		glDrawArrays(GL_TRIANGLES, 0, 36);
		m_cube_vao->UnBind();
	}

	void GLRenderer::RenderQuad()
	{
		m_quad_vao->Bind();
		glDrawArrays(GL_TRIANGLES, 0, 6);
		m_quad_vao->UnBind();
	}
}