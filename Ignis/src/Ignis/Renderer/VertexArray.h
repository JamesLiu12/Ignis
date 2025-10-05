#pragma once

#include "VertexBuffer.h"

namespace ignis
{
	class VertexArray
	{
	public:
		virtual ~VertexArray() = default;

		virtual void Bind() = 0;
		virtual void UnBind() = 0;

		virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertex_buffer) = 0;
		virtual const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const { return m_vertex_buffers; }

		static std::shared_ptr<VertexArray> Create();

	protected:
		std::vector<std::shared_ptr<VertexBuffer>> m_vertex_buffers;
	};
}