#pragma  once

#include <glm/glm.hpp>
#include "Ignis/Renderer/VertexBuffer.h"

namespace ignis
{
	class GLVertexBuffer : public VertexBuffer
	{
	public:
		GLVertexBuffer(size_t size, Usage usage);
		GLVertexBuffer(const void* vertices, size_t size, Usage usage);

		~GLVertexBuffer() override;

		void Bind() override;
		void UnBind() override;

		void SetData(const void* data, size_t size) override;

	private:
		uint32_t m_id = 0;
	};
}