#include "IBLBaker.h"
#include "GraphicsAPI.h"
#include "Ignis/Platform/OpenGL/GLIBLBaker.h"
#include "Renderer.h"

namespace ignis
{
	std::shared_ptr<IBLBaker> IBLBaker::Create(Renderer& renderer)
	{
		switch (GraphicsAPI::GetType())
		{
		case GraphicsAPI::Type::OpenGL:
			return std::make_shared<GLIBLBaker>(renderer);
		default:
			return nullptr;
		}
	}
}