#include "Application.h"
#include "Ignis/ImGui/ImGuiLayer.h"
#include "Ignis/Renderer/RendererContext.h"
#include "Ignis/Asset/VFS.h"

namespace ignis 
{
	Application::Application()
	{
		s_instance = this;

		// Initialize logging system
		Log::Init();

		// Initialize VFS
		VFS::Init();

		Log::CoreInfo("VFS initialized");

		m_window = Window::Create();
		m_window->SetEventCallback([this](EventBase& e) { OnEvent(e); });

		glfwMakeContextCurrent(static_cast<GLFWwindow*>(m_window->GetNativeWindow()));
		std::unique_ptr<RendererContext> context = RendererContext::Create();
		context->Init();

		m_renderer = Renderer::Create();

		// Initialize ImGui layer
		auto imgui_layer = ImGuiLayer::Create();
		m_imgui_layer = imgui_layer.get();
		PushOverlay(std::move(imgui_layer));

		m_subscriptions.emplace_back(
			m_dispatcher.Subscribe<WindowResizeEvent>(
				[this](WindowResizeEvent& e) { OnWindowResize(e); }
			)
		);
	}

	Application::~Application()
	{
		Log::CoreInfo("Application shutting down...");
		VFS::Shutdown();
		Log::Shutdown();
	}

	void Application::Run()
	{
		Log::CoreInfoTag("Core", "Application main loop started");

		float last_frame_time = 0.0f;

		while (m_running)
		{
			float time = static_cast<float>(glfwGetTime());
			float delta_time = time - last_frame_time;
			last_frame_time = time;

			// Update all layers
			for (auto& layer : m_layer_stack)
			{
				layer->OnUpdate(delta_time);
			}

			// ImGui rendering
			if (m_imgui_layer)
			{
				m_imgui_layer->Begin();
				
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
		if (auto* close_event = dynamic_cast<WindowCloseEvent*>(&e))
		{
			Log::CoreInfo("Window close event received in OnEvent - setting m_running to false");
			m_running = false;
			return;
		}
		
		m_dispatcher.Dispatch(e);

		for (auto it = m_layer_stack.rbegin(); it != m_layer_stack.rend(); ++it)
		{
			if (e.Handled)
				break;
			(*it)->OnEvent(e);
		}
	}

	void Application::OnWindowClose(WindowCloseEvent& e)
	{
		Log::CoreInfoTag("Core", "Window close event received");
		m_running = false;
	}

	void Application::OnWindowResize(WindowResizeEvent& e)
	{
		Log::CoreInfoTag("Core", "Window resize event received");
		m_renderer->SetViewport(0, 0, e.GetWidth(), e.GetHeight());
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