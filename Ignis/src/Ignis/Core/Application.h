#pragma once

namespace ignis {

	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
		void Close() { m_Running = false; }

	private:
		bool m_Running = true;
	};

	std::unique_ptr<Application> CreateApplication();
}