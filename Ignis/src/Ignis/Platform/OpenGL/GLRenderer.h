#pragma once

#include "Ignis/Renderer/Renderer.h"

namespace ignis
{
	class GLRenderer : public Renderer
	{
	public:
		GLRenderer() = default;
		~GLRenderer() override = default;

		void BeginFrame() override;
		void EndFrame() override;

		void SetClearColor(float r, float g, float b, float a) override;
		void SetClearColor(const glm::vec4& color) override;

		void SetViewport(int x, int y, int width, int height) override;
		void SetViewport(const glm::ivec4& viewport) override;

		void DrawIndexed(VertexArray& va) override;
		void Clear() override;
	};
}