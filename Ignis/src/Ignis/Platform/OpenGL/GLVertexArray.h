#pragma once

#include "Ignis/Renderer/VertexArray.h"

namespace ignis
{
	class GLVertexArray : public VertexArray
	{
	public:
		GLVertexArray();
		~GLVertexArray();
		
		void Bind() override;
		void UnBind() override;

		void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertex_buffer) override;

	private:
		unsigned int m_id = 0;

		std::vector<std::shared_ptr<VertexBuffer>> m_vertex_buffers;
	};
}