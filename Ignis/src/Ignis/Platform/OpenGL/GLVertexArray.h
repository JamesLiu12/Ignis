#pragma once

#include "Ignis/Renderer/VertexArray.h"
#include "Ignis/Renderer/IndexBuffer.h"

namespace ignis
{
	class GLVertexArray : public VertexArray
	{
	public:
		GLVertexArray();
		~GLVertexArray() override;
		
		void Bind() override;
		void UnBind() override;

		void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertex_buffer) override;
		const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const override { return m_vertex_buffers; }

		void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& index_buffer) override;
		const std::shared_ptr<IndexBuffer>& GetIndexBuffer() const override { return m_index_buffer; }

	private:
		uint32_t m_id = 0;

		std::vector<std::shared_ptr<VertexBuffer>> m_vertex_buffers;
		std::shared_ptr<IndexBuffer> m_index_buffer;
	};
}