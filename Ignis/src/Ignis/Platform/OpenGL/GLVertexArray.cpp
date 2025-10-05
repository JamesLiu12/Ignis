#include "GLVertexArray.h"

#include <glad/glad.h>

namespace ignis
{

	GLVertexArray::GLVertexArray()
	{
		glGenVertexArrays(1, &m_id);
		glBindVertexArray(m_id);
	}

	GLVertexArray::~GLVertexArray()
	{
		glDeleteVertexArrays(1, &m_id);
	}

	void GLVertexArray::Bind()
	{
		glBindVertexArray(m_id);
	}

	void GLVertexArray::UnBind()
	{
		glBindVertexArray(0);
	}

	void GLVertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertex_buffer)
	{
		glBindVertexArray(m_id);
		vertex_buffer->Bind();
		
		const auto& layout = vertex_buffer->GetLayout();

		for (const auto& attrib : layout.GetAttributes())
		{
			int count = Shader::DataTypeComponentCount(attrib.Type);
			switch (attrib.Type)
			{
			case Shader::DataType::Float:
			case Shader::DataType::Float2:
			case Shader::DataType::Float3:
			case Shader::DataType::Float4:
				glEnableVertexAttribArray(attrib.Index);
				glVertexAttribPointer(attrib.Index,
					count,
					GL_FLOAT,
					attrib.Normalized ? GL_TRUE : GL_FALSE,
					layout.GetStride(),
					(const void*)attrib.Offset);
				break;
			case Shader::DataType::Int:
			case Shader::DataType::Int2:
			case Shader::DataType::Int3:
			case Shader::DataType::Int4:
				glEnableVertexAttribArray(attrib.Index);
				glVertexAttribIPointer(attrib.Index,
					count,
					GL_INT,
					layout.GetStride(),
					(const void*)attrib.Offset);
				break;
			case Shader::DataType::UInt:
			case Shader::DataType::UInt2:
			case Shader::DataType::UInt3:
			case Shader::DataType::UInt4:
				glEnableVertexAttribArray(attrib.Index);
				glVertexAttribIPointer(attrib.Index,
					count,
					GL_UNSIGNED_INT,
					layout.GetStride(),
					(const void*)attrib.Offset);
				break;
			case Shader::DataType::Bool:
			case Shader::DataType::Bool2:
			case Shader::DataType::Bool3:
			case Shader::DataType::Bool4:
				glEnableVertexAttribArray(attrib.Index);
				glVertexAttribIPointer(attrib.Index,
					count,
					GL_BOOL,
					layout.GetStride(),
					(const void*)attrib.Offset);
				break;
			case Shader::DataType::Mat3:
			case Shader::DataType::Mat4:
				for (uint32_t i = 0; i < count; i++)
				{
					glEnableVertexAttribArray(attrib.Index);
					glVertexAttribPointer(attrib.Index,
						count,
						GL_FLOAT,
						attrib.Normalized ? GL_TRUE : GL_FALSE,
						layout.GetStride(),
						(const void*)(attrib.Offset + sizeof(float) * count * i));
					glVertexAttribDivisor(attrib.Index, 1);
				}
				break;
			}
		}

		m_vertex_buffers.push_back(vertex_buffer);
	}

	void GLVertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer>& index_buffer)
	{
		glBindVertexArray(m_id);
		index_buffer->Bind();
		m_index_buffer = index_buffer;
	}

}