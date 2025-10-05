#include "IndexBuffer.h"
#include "GraphicsAPI.h"
#include "Ignis/Platform/OpenGL/GLIndexBuffer.h"

namespace ignis
{
	std::shared_ptr<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t count)
	{
		switch (GraphicsAPI::GetType())
		{
		case GraphicsAPI::Type::OpenGL:
			return std::make_shared<GLIndexBuffer>(indices, count);
		default:
			return nullptr;
		}
	}
}