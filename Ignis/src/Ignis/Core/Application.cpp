#include <glad/glad.h>
#include "Application.h"
#include "Ignis/ImGui/ImGuiLayer.h"
#include "Ignis/Debug/EngineStatsPanel.h"
#include "Ignis/Core/Events/KeyEvents.h"
#include "Input.h"
#include "Ignis/Renderer/VertexBuffer.h"
#include "Ignis/Renderer/Shader.h"
#include "Ignis/Renderer/RendererContext.h"

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
	Application::Application()
	{
		s_instance = this;

		// Initialize logging system
		Log::Init();
		
		// Test logging system
		Log::CoreInfo("Ignis Engine initialized!");
		Log::CoreInfoTag("Core", "Application constructor called");
		Log::Info("Client application starting...");
		Log::WarnTag("Test", "This is a warning message with tag");
		Log::Error("This is an error message");

		m_window = Window::Create();
		m_window->SetEventCallback([this](EventBase& e) { OnEvent(e); });

		// Initialize ImGui layer
		auto imgui_layer = ImGuiLayer::Create();
		m_imgui_layer = imgui_layer.get();
		PushOverlay(std::move(imgui_layer));

		// Create debug panel
		m_debug_panel = std::make_unique<EngineStatsPanel>();
		Log::CoreInfo("Debug panel created successfully");
		Log::CoreInfo("Debug window initial state: {}", m_show_debug_window ? "VISIBLE" : "HIDDEN");

		m_subscriptions.emplace_back(
			m_dispatcher.Subscribe<WindowResizeEvent>(
				[this](WindowResizeEvent& e) { OnWindowResize(e); }
			)
		);
	}

	Application::~Application()
	{
		Log::CoreInfo("Application shutting down...");
		Log::Shutdown();
	}

	void Application::Run()
	{
		Log::CoreInfoTag("Core", "Application main loop started");

		glfwMakeContextCurrent(static_cast<GLFWwindow*>(m_window->GetNativeWindow()));
		std::unique_ptr<RendererContext> context = RendererContext::Create();
		context->Init();

		float vertices[] = {
			0.0f, 0.5f, 0.0f,
			-0.5, -0.5f, 0.0f,
			0.5f, -0.5f, 0.0f
		};

		//std::shared_ptr<VertexBuffer> vb = VertexBuffer::Create(vertices, sizeof(vertices));
		unsigned int VBO;
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		unsigned int VAO;
		glGenVertexArrays(1, &VAO);
		// 1. bind Vertex Array Object
		glBindVertexArray(VAO);
		// 2. copy our vertices array in a buffer for OpenGL to use
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// 3. then set our vertex attributes pointers
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		std::string vertex_source = R"(
				#version 330 core
				
				layout(location = 0) in vec3 aPos;
				void main()
				{
					gl_Position = vec4(aPos, 1.0);
				}
			)";

		std::string fragment_source = R"(
				#version 330 core
				
				out vec4 FragColor;
				void main()
				{
					FragColor = vec4(0.8, 0.2, 0.3, 1.0);
				}
			)";
		
		std::shared_ptr<Shader> shader = Shader::Create(vertex_source, fragment_source);

		while (m_running)
		{
			// Clear the screen buffer
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);  // Dark gray background
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			//vb->Bind();
			//shader->Bind();

			shader->Bind();
			glBindVertexArray(VAO);
			glDrawArrays(GL_TRIANGLES, 0, 3);

			// Update all layers
			for (auto& layer : m_layer_stack)
			{
				layer->OnUpdate();
			}

			if (Input::IsKeyPressed(KeyCode::A))
			{
				Log::CoreInfo("Key 'A' is currently pressed.");
			}

			if (Input::IsMouseButtonPressed(MouseButton::Left))
			{
				auto [x, y] = Input::GetMousePosition();
				Log::CoreInfo("Left mouse button is currently pressed at position ({}, {}).", x, y);
			}

			// ImGui rendering
			if (m_imgui_layer)
			{
				m_imgui_layer->Begin();
				
				// Render debug window only if enabled
				if (m_debug_panel && m_show_debug_window)
				{
					m_debug_panel->OnImGuiRender(m_show_debug_window);
				}
			
				// Render all layer ImGui
				for (auto& layer : m_layer_stack)
				{
					layer->OnImGuiRender();
				}
			
				m_imgui_layer->End();
			}
			else
			{
				Log::CoreError("ImGui layer is null!");
			}

			m_window->OnUpdate();
		}
		
		Log::CoreInfoTag("Core", "Application main loop ended");
	}

	void Application::OnEvent(EventBase& e)
	{
		// Handle window close event
		if (auto* closeEvent = dynamic_cast<WindowCloseEvent*>(&e))
		{
			Log::CoreInfo("Window close event received in OnEvent - setting m_running to false");
			m_running = false;
			return;
		}

		// Handle F1 key for debug window toggle
		if (auto* keyEvent = dynamic_cast<KeyPressedEvent*>(&e))
		{
			if (keyEvent->GetKeyCode() == 290)  // GLFW_KEY_F1 = 290
			{
				m_show_debug_window = !m_show_debug_window;
				Log::CoreInfo("Debug window toggled: {}", m_show_debug_window ? "ON" : "OFF");
			}
		}
		
		for (auto it = m_layer_stack.rbegin(); it != m_layer_stack.rend(); ++it)
		{
			if (e.Handled)
				break;
			(*it)->OnEvent(e);
		}

		m_dispatcher.Dispatch(e);
	}

	void Application::OnWindowResize(WindowResizeEvent& e)
	{
		Log::CoreInfoTag("Core", "Window resize event received");
	}

	void Application::PushLayer(std::unique_ptr<Layer> layer)
	{
		m_layer_stack.PushLayer(std::move(layer));
	}

	void Application::PushOverlay(std::unique_ptr<Layer> overlay)
	{
		m_layer_stack.PushOverlay(std::move(overlay));
	}
}