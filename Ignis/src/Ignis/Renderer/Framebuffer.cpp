#include "Framebuffer.h"
#include "GraphicsAPI.h"
#include "Ignis/Platform/OpenGL/GLFramebuffer.h"

namespace ignis
{
	std::shared_ptr<Framebuffer> Framebuffer::Create(const FrameBufferSpecs& specs)
	{
		switch (GraphicsAPI::GetType())
		{
		case GraphicsAPI::Type::OpenGL:
			return std::make_shared<GLFramebuffer>(specs);
		default:
			return nullptr;
		}
	}
}