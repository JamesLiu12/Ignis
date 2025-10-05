#include "VertexArray.h"
#include "GraphicsAPI.h"
#include "Ignis/Platform/OpenGL/GLVertexArray.h"

namespace ignis
{
	std::shared_ptr<VertexArray> VertexArray::Create()
	{
		switch (GraphicsAPI::GetType())
		{
		case GraphicsAPI::Type::OpenGL:
			return std::make_shared<GLVertexArray>();
		default:
			return nullptr;
		}
	}
}