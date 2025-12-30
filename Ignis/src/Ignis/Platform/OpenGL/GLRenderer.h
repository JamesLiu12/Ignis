#pragma once

#include "Ignis/Renderer/Renderer.h"

namespace ignis
{
	class GLRenderer : public Renderer
	{
	public:
		GLRenderer() = default;
		~GLRenderer() override = default;

		void BeginScene() override;
		void EndScene() override;

		void SetClearColor(float r, float g, float b, float a) override;
		void SetClearColor(const glm::vec4& color) override;

		void SetViewport(int x, int y, int width, int height) override;
		void SetViewport(const glm::ivec4& viewport) override;

		void DrawIndexed(VertexArray& va) override;
		void RenderMesh(const std::shared_ptr<Pipeline> pipeline, const Camera& camera, const std::shared_ptr<Mesh>& mesh, const glm::mat4& model) override;

		void Clear() override;
	};
}