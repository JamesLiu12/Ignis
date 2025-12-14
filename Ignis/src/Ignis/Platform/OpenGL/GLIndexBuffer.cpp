#include "GLIndexBuffer.h"

#include <glad/glad.h>

namespace ignis
{
	GLIndexBuffer::GLIndexBuffer(const uint32_t* indices, uint32_t size)
		: m_size(size)
	{
		glGenBuffers(1, &m_id);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, GL_STATIC_DRAW);
	}

	GLIndexBuffer::~GLIndexBuffer()
	{
		glDeleteBuffers(1, &m_id);
	}

	void GLIndexBuffer::Bind()
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
	}

	void GLIndexBuffer::Unbind()
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}