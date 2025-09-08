#pragma once

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
		void Close() { m_Running = false; }

	protected:
		Application();

	private:
		bool m_Running = true;
		inline static Application* s_instance = nullptr;
	};

	std::unique_ptr<Application> CreateApplication();
}