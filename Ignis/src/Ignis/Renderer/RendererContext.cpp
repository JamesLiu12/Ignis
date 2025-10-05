#include "RendererContext.h"
#include "GraphicsAPI.h"
#include "Ignis/Platform/OpenGL/GLRendererContext.h"
namespace ignis
{
	std::unique_ptr<RendererContext> RendererContext::Create()
	{
		switch (GraphicsAPI::GetType())
		{
		case GraphicsAPI::Type::OpenGL:
			return std::make_unique<GLRendererContext>();
		default:
			return nullptr;
		}
	}
}