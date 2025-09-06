#include "pch.h"
#include "Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <filesystem>

namespace ignis {

	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
	std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

	void Log::Init()
	{
		// Create "logs" directory if doesn't exist
		std::string logsDirectory = "logs";
		if (!std::filesystem::exists(logsDirectory))
			std::filesystem::create_directories(logsDirectory);

		// Create sinks for core logger (engine)
		std::vector<spdlog::sink_ptr> ignisSinks =
		{
			std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/IGNIS.log", true),
			std::make_shared<spdlog::sinks::stdout_color_sink_mt>()
		};

		// Create sinks for client logger (application)
		std::vector<spdlog::sink_ptr> appSinks =
		{
			std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/APP.log", true),
			std::make_shared<spdlog::sinks::stdout_color_sink_mt>()
		};

		// Set patterns for file sinks
		ignisSinks[0]->set_pattern("[%T] [%l] %n: %v");
		appSinks[0]->set_pattern("[%T] [%l] %n: %v");

		// Set patterns for console sinks (with colors)
		ignisSinks[1]->set_pattern("%^[%T] %n: %v%$");
		appSinks[1]->set_pattern("%^[%T] %n: %v%$");

		// Create loggers
		s_CoreLogger = std::make_shared<spdlog::logger>("IGNIS", ignisSinks.begin(), ignisSinks.end());
		s_CoreLogger->set_level(spdlog::level::trace);

		s_ClientLogger = std::make_shared<spdlog::logger>("APP", appSinks.begin(), appSinks.end());
		s_ClientLogger->set_level(spdlog::level::trace);

		// Initialize default tag settings
		s_EnabledTags[""] = TagDetails{ true, Level::Trace };
		s_EnabledTags["Core"] = TagDetails{ true, Level::Trace };
		s_EnabledTags["Renderer"] = TagDetails{ true, Level::Info };
		s_EnabledTags["Events"] = TagDetails{ true, Level::Info };
		s_EnabledTags["Input"] = TagDetails{ true, Level::Info };
		s_EnabledTags["Audio"] = TagDetails{ true, Level::Info };
		s_EnabledTags["Physics"] = TagDetails{ true, Level::Warn };
		s_EnabledTags["Scene"] = TagDetails{ true, Level::Info };
		s_EnabledTags["Memory"] = TagDetails{ true, Level::Error };
	}

	void Log::Shutdown()
	{
		s_ClientLogger.reset();
		s_CoreLogger.reset();
		spdlog::drop_all();
	}

}
