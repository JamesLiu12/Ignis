#pragma once

#include <memory>

namespace Ignis {

	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
	};

	std::unique_ptr<Application> CreateApplication();
}