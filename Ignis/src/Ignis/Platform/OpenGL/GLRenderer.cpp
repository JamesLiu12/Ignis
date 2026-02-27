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
	}

	GLRenderer::GLRenderer()
	{
		m_shader_library = std::make_unique<ShaderLibrary>();

		m_shader_library->Load("resources://shaders/example.glsl");

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
			material->GetShader()->Bind();

			material->Set("view", m_camera->GetView());
			material->Set("projection", m_camera->GetProjection());

			// TODO hard coded to be refactored
			material->Set("model", model);
			material->Set("viewPos", m_camera->GetPosition());

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

		RenderCube();

		glDepthFunc(GL_LESS);
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