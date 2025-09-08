#pragma once

namespace ignis {

	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
		void Close() { m_running = false; }

	private:
		bool m_running = true;
	};

	std::unique_ptr<Application> CreateApplication();
}