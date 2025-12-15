#include "GLRenderer.h"
#include "Ignis/Renderer/Shader.h"

#include <glad/glad.h>

namespace ignis
{
	void GLRenderer::BeginScene()
	{
		// TODO
		glEnable(GL_DEPTH_TEST);
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

	void GLRenderer::RenderMesh(const std::shared_ptr<Mesh>& mesh, Shader& shader)
	{
		auto vao = mesh->GetVertexArray();
		vao->Bind();

		const auto& materials = mesh->GetMaterials();

		for (const auto& sm : mesh->GetSubmeshes())
		{
			const auto& mat = materials[sm.MaterialIndex];

			auto diffuse = mat.GetTexture(ignis::MaterialType::Diffuse);
			auto specular = mat.GetTexture(ignis::MaterialType::Specular);
			auto normal = mat.GetTexture(ignis::MaterialType::Normal);

			//if (!diffuse)  diffuse = m_whiteTexture;
			//if (!specular) specular = m_whiteTexture;
			//if (!normal)   normal = m_flatNormalTexture;

			diffuse->Bind(0);
			specular->Bind(1);
			normal->Bind(2);

			shader.Set("material.diffuse", 0);
			shader.Set("material.specular", 1);
			shader.Set("material.normal", 2);

			shader.Set("material.shininess", 32.0f);
			shader.Set("material.hasNormal", normal ? 1 : 0);

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