#include "GLFWWindow.h"
#include "Ignis/Events/Event.h"
#include "Ignis/Events/WindowEvents.h"

namespace ignis
{
	GlfwWindow::GlfwWindow(const WindowProps& props)
	{
		m_data.Title = props.Title;
		m_data.Width = props.Width;
		m_data.Height = props.Height;
		m_data.VSync = props.VSync;

		if (!glfwInit())
		{
			Log::CoreError("Could not initialize GLFW!");
			return;
		}

		m_window = glfwCreateWindow((int)props.Width, (int)props.Height, m_data.Title.c_str(), nullptr, nullptr);

		if (!m_window)
		{
			Log::CoreError("Failed to create GLFW window!");
			glfwTerminate();
			return;
		}
		glfwMakeContextCurrent(m_window);
		glfwSetWindowUserPointer(m_window, &m_data);
		glfwSwapInterval(1);

		glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int width, int height)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				data.Width = width;
				data.Height = height;
				WindowResizeEvent event(width, height);
				data.EventCallback(event);
			});

		glfwSetWindowCloseCallback(m_window, [](GLFWwindow* window)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				WindowCloseEvent event;
				data.EventCallback(event);
			});
		Log::CoreInfoTag("GLFW", "Created window '{0}' ({1}, {2})", m_data.Title, m_data.Width, m_data.Height);
	}

	GlfwWindow::~GlfwWindow() {
		Shutdown();
	}

	void GlfwWindow::OnUpdate()
	{
		glfwPollEvents();
		glfwSwapBuffers(m_window);
	}

	void GlfwWindow::SetVSync(bool enabled)
	{
		glfwSwapInterval(enabled);
		m_data.VSync = enabled;
	}

	void GlfwWindow::Shutdown()
	{
		glfwDestroyWindow(m_window);
		m_window = nullptr;
	}
}