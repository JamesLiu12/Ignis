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

	void GLRenderer::RenderMesh(const std::shared_ptr<Pipeline> pipeline, const std::shared_ptr<Mesh>& mesh)
	{
		auto vao = mesh->GetVertexArray();
		vao->Bind();

		const auto& materials = mesh->GetMaterials();

		for (const auto& sm : mesh->GetSubmeshes())
		{
			const auto& material_data = materials[sm.MaterialIndex];

			auto material = pipeline->CreateMaterial(material_data);

			// TODO Use Material

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