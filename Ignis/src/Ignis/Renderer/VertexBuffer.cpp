#include "VertexBuffer.h"
#include "GraphicsAPI.h"
#include "Ignis/Platform/OpenGL/GLVertexBuffer.h"

namespace ignis
{
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

	std::shared_ptr<ignis::VertexBuffer> VertexBuffer::Create(float* vertices, size_t size, Usage usage)
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