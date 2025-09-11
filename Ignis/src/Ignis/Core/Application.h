#pragma once

#include "Ignis/Events/Event.h"
#include "Window.h"

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
		
		static std::unique_ptr<Application> Create();

	protected:
		Application();

	private:
		bool m_running = true;
		inline static Application* s_instance = nullptr;

		std::unique_ptr<Window> m_window;
		EventDispatcher m_dispatcher;
		std::vector<EventDispatcher::Subscription> m_subscriptions;
	};
}