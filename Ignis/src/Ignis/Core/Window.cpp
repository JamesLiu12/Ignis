#include "Window.h"
#include "Ignis/Core/Events/WindowEvents.h"
#include "Ignis/Core/Events/KeyEvents.h"
#include "Ignis/Core/Events/MouseEvents.h"


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

		// Initialize framebuffer size
		int fb_width, fb_height;
		glfwGetFramebufferSize(m_window, &fb_width, &fb_height);
		m_data.FramebufferWidth = static_cast<uint32_t>(fb_width);
		m_data.FramebufferHeight = static_cast<uint32_t>(fb_height);

		// Initialize content scale (DPI scaling factor)
		float xscale, yscale;
		glfwGetWindowContentScale(m_window, &xscale, &yscale);
		m_data.ContentScaleX = xscale;
		m_data.ContentScaleY = yscale;

		SetUpCallbacks();

		Log::CoreInfoTag("GLFW", "Created window '{0}' ({1}x{2})", m_data.Title, m_data.Width, m_data.Height);
		Log::CoreInfoTag("GLFW", "Framebuffer size: {0}x{1}", m_data.FramebufferWidth, m_data.FramebufferHeight);
		Log::CoreInfoTag("GLFW", "Content scale: {0}x{1}", m_data.ContentScaleX, m_data.ContentScaleY);
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

	void Window::SetCursorMode(int mode)
	{
		m_cursor_mode = mode;
		glfwSetInputMode(m_window, GLFW_CURSOR, mode);

		if (glfwRawMouseMotionSupported())
		{
			bool raw = (mode == GLFW_CURSOR_DISABLED);
			glfwSetInputMode(m_window, GLFW_RAW_MOUSE_MOTION, raw ? GLFW_TRUE : GLFW_FALSE);
		}
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

		// Framebuffer size callback tracks actual pixel buffer size changes
		glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow* window, int width, int height)
			{
				WindowData* data = (WindowData*)glfwGetWindowUserPointer(window);
				data->FramebufferWidth = static_cast<uint32_t>(width);
				data->FramebufferHeight = static_cast<uint32_t>(height);
				Log::CoreInfoTag("GLFW", "Framebuffer resized: {0}x{1}", width, height);
				
				// Dispatch resize event so renderer can update viewport
				WindowResizeEvent event(width, height);
				data->EventCallback(event);
			});
	}

	std::unique_ptr<Window> Window::Create(const WindowSpecs& specs)
	{
		return std::make_unique<Window>(specs);
	}
}