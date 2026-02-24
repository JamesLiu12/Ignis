#include "VertexBuffer.h"
#include "GraphicsAPI.h"
#include "Ignis/Platform/OpenGL/GLVertexBuffer.h"

namespace ignis
{
	VertexBuffer::Layout::Layout(const std::initializer_list<Attribute>& attributes)
		: m_attributes(attributes)
	{
		uint32_t offset = 0;
		m_stride = 0;
		for (auto& attribute : m_attributes) {
			attribute.Offset = offset;
			offset += Shader::DataTypeSize(attribute.Type);
		}
		m_stride = offset;
	}

	std::shared_ptr<VertexBuffer> VertexBuffer::Create(size_t size, Usage usage)
	{
		switch (GraphicsAPI::GetType())
		{
		case GraphicsAPI::Type::OpenGL:
			return std::make_shared<GLVertexBuffer>(size, usage);
		default:
			return nullptr;
		}
	}

	std::shared_ptr<VertexBuffer> VertexBuffer::Create(const void* vertices, size_t size, Usage usage)
	{
		switch (GraphicsAPI::GetType())
		{
		case GraphicsAPI::Type::OpenGL:
			return std::make_shared<GLVertexBuffer>(vertices, size, usage);
		default:
			return nullptr;
		}
	}
}