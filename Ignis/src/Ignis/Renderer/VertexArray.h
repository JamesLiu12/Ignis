#pragma once

#include "VertexBuffer.h"
#include "IndexBuffer.h"

namespace ignis
{
	class VertexArray
	{
	public:
		virtual ~VertexArray() = default;

		virtual void Bind() = 0;
		virtual void UnBind() = 0;

		virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertex_buffer) = 0;
		virtual const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const = 0;

		virtual void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& index_buffer) = 0;
		virtual const std::shared_ptr<IndexBuffer>& GetIndexBuffer() const = 0;

		static std::shared_ptr<VertexArray> Create();
	};
}