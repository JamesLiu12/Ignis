#include "GLRendererContext.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace ignis
{
	void GLRendererContext::Init()
	{
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		if (!status)
		{
			Log::CoreError("Failed to initialize GLAD!");
			return;
		}
	}
}