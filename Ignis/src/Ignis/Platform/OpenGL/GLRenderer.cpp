#include "GLRenderer.h"
#include "Ignis/Renderer/Shader.h"

#include <glad/glad.h>

namespace ignis
{
	GLRenderer::GLRenderer()
	{
		float skybox_vertices[] = {       
			-1.0f,  1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			-1.0f,  1.0f, -1.0f,
			 1.0f,  1.0f, -1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			 1.0f, -1.0f,  1.0f
		};

		m_skybox_vao = VertexArray::Create();
		std::shared_ptr<VertexBuffer> skybox_vbo = VertexBuffer::Create(skybox_vertices, sizeof(skybox_vertices));

		VertexBuffer::Layout layout
		{
			{ 0, Shader::DataType::Float3 }
		};
		skybox_vbo->SetLayout(layout);
		m_skybox_vao->AddVertexBuffer(skybox_vbo);

		float quad_vertices[] = {
			// positions   // texCoords
			-1.0f,  1.0f,  0.0f, 1.0f,
			-1.0f, -1.0f,  0.0f, 0.0f,
			 1.0f, -1.0f,  1.0f, 0.0f,

			-1.0f,  1.0f,  0.0f, 1.0f,
			 1.0f, -1.0f,  1.0f, 0.0f,
			 1.0f,  1.0f,  1.0f, 1.0f
		};

		m_quad_vao = VertexArray::Create();
		std::shared_ptr<VertexBuffer> quad_vbo = VertexBuffer::Create(quad_vertices, sizeof(quad_vertices));

		VertexBuffer::Layout quad_layout
		{
			{ 0, Shader::DataType::Float2 }, // position
			{ 1, Shader::DataType::Float2 }  // texCoord
		};
		quad_vbo->SetLayout(quad_layout);
		m_quad_vao->AddVertexBuffer(quad_vbo);

		m_screen_shader = Shader::Create("screen",
			// Vertex Shader
			"#version 330 core\n\
			layout(location = 0) in vec2 aPos;\
			layout(location = 1) in vec2 aTexCoords;\
			\
			out vec2 TexCoords;\
			\
			void main()\
			{\
				TexCoords = aTexCoords;\
				gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);\
			}",
			//Fragment Shader
			"#version 330 core\n\
			out vec4 FragColor;\
			\
			in vec2 TexCoords;\
			\
			uniform sampler2D screenTexture;\
			\
			void main()\
			{\
				FragColor = texture(screenTexture, TexCoords);\
			}"
		);
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
		m_skybox_vao->Bind();
		glDrawArrays(GL_TRIANGLES, 0, 36);
		m_skybox_vao->UnBind();

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
}