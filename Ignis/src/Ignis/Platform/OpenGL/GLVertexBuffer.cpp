#include "GLVertexBuffer.h"

#include <glad/glad.h>

namespace ignis
{
	GLVertexBuffer::GLVertexBuffer(size_t size, Usage usage)
	{
		glGenBuffers(1, &m_id);
		glBindBuffer(GL_ARRAY_BUFFER, m_id);
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, usage == Usage::Dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
	}

	GLVertexBuffer::GLVertexBuffer(const void* vertices, size_t size, Usage usage)
	{
		glGenBuffers(1, &m_id);
		glBindBuffer(GL_ARRAY_BUFFER, m_id);
		glBufferData(GL_ARRAY_BUFFER, size, vertices, usage == Usage::Dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
	}

	GLVertexBuffer::~GLVertexBuffer()
	{
		glDeleteBuffers(1, &m_id);
	}

	void GLVertexBuffer::Bind()
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_id);
	}

	void GLVertexBuffer::UnBind()
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void GLVertexBuffer::SetData(const void* data, size_t size)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_id);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
	}
}