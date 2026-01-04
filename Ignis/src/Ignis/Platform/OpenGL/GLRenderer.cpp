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
	}

	void GLRenderer::BeginScene(std::shared_ptr<Pipeline> pipeline, std::shared_ptr<Scene> scene, std::shared_ptr<Camera> camera)
	{
		m_pipeline = pipeline;
		m_scene = scene;
		m_camera = camera;

		glEnable(GL_DEPTH_TEST);

		scene->OnRender();
		// TODO: Move this to SceneRenderer
		m_light_environment = scene->m_light_environment;
		m_scene_environment = scene->m_scene_environment;
		m_environment_settings = scene->m_environment_settings;
	}

	void GLRenderer::EndScene()
	{
		auto material = m_pipeline->CreateSkyboxMaterial(m_scene_environment);
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

	void GLRenderer::RenderMesh(const std::shared_ptr<Mesh>& mesh, const glm::mat4& model)
	{
		auto vao = mesh->GetVertexArray();
		vao->Bind();

		const auto& materials_data = mesh->GetMaterialsData();

		for (const auto& sm : mesh->GetSubmeshes())
		{
			const auto& material_data = materials_data[sm.MaterialIndex];

			// TODO a performance bottleneck?
			auto material = m_pipeline->CreateMaterial(material_data);
			m_pipeline->ApplyEnvironment(*material, m_scene_environment, m_environment_settings, m_light_environment);
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

	void GLRenderer::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
}