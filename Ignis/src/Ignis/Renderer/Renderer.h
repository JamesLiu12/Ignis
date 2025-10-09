#pragma once

#include <glm/glm.hpp>
#include "VertexArray.h"
#include "Camera.h"

namespace ignis
{	
	class Renderer
	{
	public:
		virtual ~Renderer() = default;

		virtual void BeginScene() = 0;
		virtual void EndScene() = 0;

		virtual void SetClearColor(float r, float g, float b, float a) = 0;
		virtual void SetClearColor(const glm::vec4& color) = 0;
		
		virtual void SetViewport(int x, int y, int width, int height) = 0;
		virtual void SetViewport(const glm::ivec4& viewport) = 0;

		virtual void DrawIndexed(VertexArray& va) = 0;

		virtual void Clear() = 0;

		static std::unique_ptr<Renderer> Create();
	};
}