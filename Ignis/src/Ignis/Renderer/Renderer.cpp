#include "Renderer.h"
#include "GraphicsAPI.h"
#include "Ignis/Platform/OpenGL/GLRenderer.h"

namespace ignis
{
	std::unique_ptr<Renderer> Renderer::Create()
	{
		switch (GraphicsAPI::GetType())
		{
		case GraphicsAPI::Type::OpenGL:
			return std::make_unique<GLRenderer>();
		default:
			return nullptr;
		}
	}
}