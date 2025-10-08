#pragma once

#include "Ignis/Core/Events/Event.h"
#include "Window.h"
#include "LayerStack.h"
#include "Ignis/Core/Events/WindowEvents.h"
#include "Ignis/Physics/PhysicsWorld.h"
#include "Ignis/Debug/PhysicsDebugPanel.h"

namespace ignis {

	class Application
	{
	public:
		virtual ~Application();

		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;
		Application(Application&&) = delete;
		Application& operator=(Application&&) = delete;

		static Application& Get() { return *s_instance; }

		void Run();
		void Close() { m_running = false; }

		void OnEvent(EventBase& e);

		void OnWindowClose(WindowCloseEvent& e);
		void OnWindowResize(WindowResizeEvent& e);

		void PushLayer(std::unique_ptr<Layer> layer);
		void PushOverlay(std::unique_ptr<Layer> overlay);
		Window& GetWindow() { return *m_window; }
		
		static std::unique_ptr<Application> Create();

	protected:
		Application();

	private:
		bool m_running = true;
		inline static Application* s_instance = nullptr;

		std::unique_ptr<Window> m_window;
		EventDispatcher m_dispatcher;
		LayerStack m_layer_stack;
		class ImGuiLayer* m_imgui_layer;
		std::unique_ptr<class EngineStatsPanel> m_debug_panel;
		bool m_show_debug_window = true;  // Set to true to show debug window by default

		void CreatePhysicsTestScene();
		std::unique_ptr<class PhysicsWorld> m_physicsWorld;
		std::unique_ptr<class PhysicsDebugPanel> m_physicsDebugPanel;
		bool m_show_physics_debug = true;
		std::vector<EventDispatcher::Subscription> m_subscriptions;
	};
}