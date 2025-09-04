#pragma once

namespace ignis {

	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
	};

	std::unique_ptr<Application> CreateApplication();
}