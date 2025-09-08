#include "Window.h"
#include "Ignis/Platform/GLFW/GLFWWindow.h"

namespace ignis
{
	std::unique_ptr<Window> Window::Create(const WindowProps& props)
	{
		// Currently, we only have a GLFW implementation
		return std::make_unique<GlfwWindow>(props);
	}
}