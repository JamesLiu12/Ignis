#include "Window.h"
#include "Ignis/Events/WindowEvents.h"
#include "Ignis/Events/KeyEvents.h"
#include "Ignis/Events/MouseEvents.h"

// OpenGL headers
#ifdef __APPLE__
	#define GL_SILENCE_DEPRECATION
	#define GL_GLEXT_PROTOTYPES
	#define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
	#include <OpenGL/gl3.h>
#else
	#include <GL/gl.h>
#endif

namespace ignis
{
	

	Window::Window(const WindowSpecs& props)
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

		// Set OpenGL version hints for macOS compatibility
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required on macOS
#endif

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

		SetUpCallbacks();

		Log::CoreInfoTag("GLFW", "Created window '{0}' ({1}, {2})", m_data.Title, m_data.Width, m_data.Height);
	}

	Window::~Window() {
		Shutdown();
	}

	void Window::OnUpdate()
	{
		glfwPollEvents();
		glfwSwapBuffers(m_window);
	}

	void Window::SetVSync(bool enabled)
	{
		glfwSwapInterval(enabled);
		m_data.VSync = enabled;
	}

	void Window::Shutdown()
	{
		glfwDestroyWindow(m_window);
		m_window = nullptr;
	}

	void Window::SetUpCallbacks()
	{
		glfwSetWindowCloseCallback(m_window, [](GLFWwindow* window)
			{
				WindowData* data = (WindowData*)glfwGetWindowUserPointer(window);
				Log::CoreInfo("GLFW window close callback triggered");
				WindowCloseEvent event;
				data->EventCallback(event);
			});

		glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int width, int height)
			{
				WindowData* data = (WindowData*)glfwGetWindowUserPointer(window);
				data->Width = width;
				data->Height = height;
				WindowResizeEvent event(width, height);
				data->EventCallback(event);
			});

		glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
			{
				WindowData* data = (WindowData*)glfwGetWindowUserPointer(window);

				switch (action)
				{
				case GLFW_PRESS:
				{
					KeyPressedEvent event(key, false);
					data->EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent event(key);
					data->EventCallback(event);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent event(key, true);
					data->EventCallback(event);
					break;
				}
				default:
					break;
				}
			});

		glfwSetCharCallback(m_window, [](GLFWwindow* window, unsigned int keycode)
			{
				WindowData* data = (WindowData*)glfwGetWindowUserPointer(window);
				KeyTypedEvent event(keycode);
				data->EventCallback(event);
			});

		glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xPos, double yPos)
			{
				WindowData* data = (WindowData*)glfwGetWindowUserPointer(window);
				MouseMovedEvent event(xPos, yPos);
				data->EventCallback(event);
			});

		glfwSetScrollCallback(m_window, [](GLFWwindow* window, double xOffset, double yOffset)
			{
				WindowData* data = (WindowData*)glfwGetWindowUserPointer(window);
				MouseScrolledEvent event((float)xOffset, (float)yOffset);
				data->EventCallback(event);
			});

		glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int button, int action, int mods)
			{
				WindowData* data = (WindowData*)glfwGetWindowUserPointer(window);
				switch (action)
				{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent event(button);
					data->EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event(button);
					data->EventCallback(event);
					break;
				}
				default:
					break;
				}
			});
	}

	std::unique_ptr<Window> Window::Create(const WindowSpecs& specs)
	{
		return std::make_unique<Window>(specs);
	}
}