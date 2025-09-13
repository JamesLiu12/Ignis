#include "Input.h"
#include "Application.h"

namespace ignis
{
	bool Input::IsKeyPressed(KeyCode key)
	{
		Window& window = Application::Get().GetWindow();
		int state = glfwGetKey(window.GetNativeWindow(), static_cast<int>(key));
		return state == GLFW_PRESS;
	}

	bool Input::IsKeyReleased(KeyCode key)
	{
		Window& window = Application::Get().GetWindow();
		int state = glfwGetKey(window.GetNativeWindow(), static_cast<int>(key));
		return state == GLFW_RELEASE;
	}

	bool Input::IsMouseButtonPressed(MouseButton button)
	{
		Window& window = Application::Get().GetWindow();
		int state = glfwGetMouseButton(window.GetNativeWindow(), static_cast<int>(button));
		return state == GLFW_PRESS;
	}

	bool Input::IsMouseButtonReleased(MouseButton button)
	{
		Window& window = Application::Get().GetWindow();
		int state = glfwGetMouseButton(window.GetNativeWindow(), static_cast<int>(button));
		return state == GLFW_RELEASE;
	}

	std::pair<double, double> Input::GetMousePosition()
	{
		Window& window = Application::Get().GetWindow();
		double xpos, ypos;
		glfwGetCursorPos(window.GetNativeWindow(), &xpos, &ypos);
		return { xpos, ypos };
	}

	double Input::GetMouseX()
	{
		return GetMousePosition().first;
	}

	double Input::GetMouseY()
	{
		return GetMousePosition().second;
	}
}