#include "GLRenderer.h"
#include "Ignis/Renderer/Shader.h"

#include <glad/glad.h>

namespace ignis
{
	void GLRenderer::BeginScene(Scene& scene, const Camera& camera)
	{
		glEnable(GL_DEPTH_TEST);

		scene.OnRender();
		// TODO: Move this to SceneRenderer
		m_light_environment = scene.m_light_environment;
	}
	void GLRenderer::EndScene()
	{
		// TODO
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

	void UploadLightsToMaterial(std::shared_ptr<Material> material, LightEnvironment light_environment)
	{
		material->Set("numDirectionalLights", (int)light_environment.DirectionalLights.size());
		for (size_t i = 0; i < light_environment.DirectionalLights.size(); i++)
		{
			std::string base = "directionalLights[" + std::to_string(i) + "]";
			material->Set(base + ".direction", light_environment.DirectionalLights[i].Direction);
			material->Set(base + ".radiance", light_environment.DirectionalLights[i].Radiance);
		}

		material->Set("numPointLights", (int)light_environment.PointLights.size());
		for (size_t i = 0; i < light_environment.PointLights.size(); i++)
		{
			std::string base = "pointLights[" + std::to_string(i) + "]";
			material->Set(base + ".position", light_environment.PointLights[i].Position);
			material->Set(base + ".radiance", light_environment.PointLights[i].Radiance);
			material->Set(base + ".constant", light_environment.PointLights[i].Constant);
			material->Set(base + ".linear", light_environment.PointLights[i].Linear);
			material->Set(base + ".quadratic", light_environment.PointLights[i].Quadratic);
		}

		material->Set("numSpotLights", (int)light_environment.SpotLights.size());
		for (size_t i = 0; i < light_environment.SpotLights.size(); i++)
		{
			std::string base = "spotLights[" + std::to_string(i) + "]";
			material->Set(base + ".position", light_environment.SpotLights[i].Position);
			material->Set(base + ".direction", light_environment.SpotLights[i].Direction);
			material->Set(base + ".radiance", light_environment.SpotLights[i].Radiance);
			material->Set(base + ".constant", light_environment.SpotLights[i].Constant);
			material->Set(base + ".linear", light_environment.SpotLights[i].Linear);
			material->Set(base + ".quadratic", light_environment.SpotLights[i].Quadratic);
			material->Set(base + ".cutOff", light_environment.SpotLights[i].CutOff);
			material->Set(base + ".outerCutOff", light_environment.SpotLights[i].OuterCutOff);
		}
	}

	void GLRenderer::RenderMesh(const std::shared_ptr<Pipeline> pipeline, const Camera& camera, const std::shared_ptr<Mesh>& mesh, const glm::mat4& model)
	{
		auto vao = mesh->GetVertexArray();
		vao->Bind();

		const auto& materials_data = mesh->GetMaterialsData();

		for (const auto& sm : mesh->GetSubmeshes())
		{
			const auto& material_data = materials_data[sm.MaterialIndex];

			// TODO a performance bottleneck?
			auto material = pipeline->CreateMaterial(material_data);
			material->GetShader()->Bind();

			material->Set("view", camera.GetView());
			material->Set("projection", camera.GetProjection());

			// TODO hard coded to be refactored
			material->Set("model", model);
			material->Set("viewPos", camera.GetPosition());

			UploadLightsToMaterial(material, m_light_environment);

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