#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

namespace ignis {

	class Log
	{
	public:
		enum class Type : uint8_t
		{
			Core = 0, Client = 1
		};

		enum class Level : uint8_t
		{
			Trace = 0, Info, Warn, Error, Fatal
		};

		struct tag_details
		{
			bool enabled = true;
			Level level_filter = Level::Trace;
		};

	public:
		static void Init();
		static void Shutdown();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return m_core_logger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return m_client_logger; }

		static bool HasTag(const std::string& tag) { return m_enabled_tags.find(tag) != m_enabled_tags.end(); }
		static std::map<std::string, tag_details>& EnabledTags() { return m_enabled_tags; }

		// TODO: Add SetTagLevel method for runtime tag configuration
		// TODO: Add PrintAssertMessage methods for debugging assertions
		// TODO: Add EditorConsoleLogger for level editor
		// TODO: Add default tag settings map for better organization
		// TODO: Implement log file rotation and cleanup

		template<typename... Args>
		static void PrintMessage(Log::Type type, Log::Level level, spdlog::format_string_t<Args...> fmt, Args&&... args);

		template<typename... Args>
		static void PrintMessageTag(Log::Type type, Log::Level level, const std::string& tag, spdlog::format_string_t<Args...> fmt, Args&&... args);

		// Enum utils
		static const char* LevelToString(Level level)
		{
			switch (level)
			{
				case Level::Trace: return "Trace";
				case Level::Info:  return "Info";
				case Level::Warn:  return "Warn";
				case Level::Error: return "Error";
				case Level::Fatal: return "Fatal";
			}
			return "";
		}

		static Level LevelFromString(std::string_view string)
		{
			if (string == "Trace") return Level::Trace;
			if (string == "Info")  return Level::Info;
			if (string == "Warn")  return Level::Warn;
			if (string == "Error") return Level::Error;
			if (string == "Fatal") return Level::Fatal;
			return Level::Trace;
		}

	private:
		static std::shared_ptr<spdlog::logger> m_core_logger;
		static std::shared_ptr<spdlog::logger> m_client_logger;
		// TODO: Add EditorConsoleLogger for level editor
		
		static std::map<std::string, tag_details> m_enabled_tags;
		// TODO: Add default tag settings map for better organization
	};

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tagged logs (prefer these!)                                                                                      //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Core logging
#define IG_CORE_TRACE_TAG(tag, ...) ::ignis::Log::PrintMessageTag(::ignis::Log::Type::Core, ::ignis::Log::Level::Trace, tag, __VA_ARGS__)
#define IG_CORE_INFO_TAG(tag, ...)  ::ignis::Log::PrintMessageTag(::ignis::Log::Type::Core, ::ignis::Log::Level::Info, tag, __VA_ARGS__)
#define IG_CORE_WARN_TAG(tag, ...)  ::ignis::Log::PrintMessageTag(::ignis::Log::Type::Core, ::ignis::Log::Level::Warn, tag, __VA_ARGS__)
#define IG_CORE_ERROR_TAG(tag, ...) ::ignis::Log::PrintMessageTag(::ignis::Log::Type::Core, ::ignis::Log::Level::Error, tag, __VA_ARGS__)
#define IG_CORE_FATAL_TAG(tag, ...) ::ignis::Log::PrintMessageTag(::ignis::Log::Type::Core, ::ignis::Log::Level::Fatal, tag, __VA_ARGS__)

// Client logging
#define IG_TRACE_TAG(tag, ...) ::ignis::Log::PrintMessageTag(::ignis::Log::Type::Client, ::ignis::Log::Level::Trace, tag, __VA_ARGS__)
#define IG_INFO_TAG(tag, ...)  ::ignis::Log::PrintMessageTag(::ignis::Log::Type::Client, ::ignis::Log::Level::Info, tag, __VA_ARGS__)
#define IG_WARN_TAG(tag, ...)  ::ignis::Log::PrintMessageTag(::ignis::Log::Type::Client, ::ignis::Log::Level::Warn, tag, __VA_ARGS__)
#define IG_ERROR_TAG(tag, ...) ::ignis::Log::PrintMessageTag(::ignis::Log::Type::Client, ::ignis::Log::Level::Error, tag, __VA_ARGS__)
#define IG_FATAL_TAG(tag, ...) ::ignis::Log::PrintMessageTag(::ignis::Log::Type::Client, ::ignis::Log::Level::Fatal, tag, __VA_ARGS__)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Core Logging
#define IG_CORE_TRACE(...)  ::ignis::Log::PrintMessage(::ignis::Log::Type::Core, ::ignis::Log::Level::Trace, __VA_ARGS__)
#define IG_CORE_INFO(...)   ::ignis::Log::PrintMessage(::ignis::Log::Type::Core, ::ignis::Log::Level::Info, __VA_ARGS__)
#define IG_CORE_WARN(...)   ::ignis::Log::PrintMessage(::ignis::Log::Type::Core, ::ignis::Log::Level::Warn, __VA_ARGS__)
#define IG_CORE_ERROR(...)  ::ignis::Log::PrintMessage(::ignis::Log::Type::Core, ::ignis::Log::Level::Error, __VA_ARGS__)
#define IG_CORE_FATAL(...)  ::ignis::Log::PrintMessage(::ignis::Log::Type::Core, ::ignis::Log::Level::Fatal, __VA_ARGS__)

// Client Logging
#define IG_TRACE(...)   ::ignis::Log::PrintMessage(::ignis::Log::Type::Client, ::ignis::Log::Level::Trace, __VA_ARGS__)
#define IG_INFO(...)    ::ignis::Log::PrintMessage(::ignis::Log::Type::Client, ::ignis::Log::Level::Info, __VA_ARGS__)
#define IG_WARN(...)    ::ignis::Log::PrintMessage(::ignis::Log::Type::Client, ::ignis::Log::Level::Warn, __VA_ARGS__)
#define IG_ERROR(...)   ::ignis::Log::PrintMessage(::ignis::Log::Type::Client, ::ignis::Log::Level::Error, __VA_ARGS__)
#define IG_FATAL(...)   ::ignis::Log::PrintMessage(::ignis::Log::Type::Client, ::ignis::Log::Level::Fatal, __VA_ARGS__)

namespace ignis {

	template<typename... Args>
	void Log::PrintMessage(Log::Type type, Log::Level level, spdlog::format_string_t<Args...> fmt, Args&&... args)
	{
		auto detail = m_enabled_tags[""];
		if (detail.enabled && detail.level_filter <= level)
		{
			auto logger = (type == Type::Core) ? GetCoreLogger() : GetClientLogger();
			switch (level)
			{
			case Level::Trace:
				logger->trace(fmt, std::forward<Args>(args)...);
				break;
			case Level::Info:
				logger->info(fmt, std::forward<Args>(args)...);
				break;
			case Level::Warn:
				logger->warn(fmt, std::forward<Args>(args)...);
				break;
			case Level::Error:
				logger->error(fmt, std::forward<Args>(args)...);
				break;
			case Level::Fatal:
				logger->critical(fmt, std::forward<Args>(args)...);
				break;
			}
		}
	}

	template<typename... Args>
	void Log::PrintMessageTag(Log::Type type, Log::Level level, const std::string& tag, spdlog::format_string_t<Args...> fmt, Args&&... args)
	{
		auto detail = m_enabled_tags[tag];
		if (detail.enabled && detail.level_filter <= level)
		{
			auto logger = (type == Type::Core) ? GetCoreLogger() : GetClientLogger();
			std::string formatted = fmt::format(fmt, std::forward<Args>(args)...);
			switch (level)
			{
				case Level::Trace:
					logger->trace("[{}] {}", tag, formatted);
					break;
				case Level::Info:
					logger->info("[{}] {}", tag, formatted);
					break;
				case Level::Warn:
					logger->warn("[{}] {}", tag, formatted);
					break;
				case Level::Error:
					logger->error("[{}] {}", tag, formatted);
					break;
				case Level::Fatal:
					logger->critical("[{}] {}", tag, formatted);
					break;
			}
		}
	}

}
