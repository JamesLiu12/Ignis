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

		// Compile-time log level filtering (set this based on build configuration)
		static constexpr Level compile_time_min_level = Level::Trace; // Debug: Trace, Release: Info

		// Modern C++ logging interface - Core logging functions
		template<typename... Args>
		static constexpr void core_trace(spdlog::format_string_t<Args...> fmt, Args&&... args) {
			if constexpr (Level::Trace >= compile_time_min_level) {
				print_message_impl(Type::Core, Level::Trace, fmt, std::forward<Args>(args)...);
			}
		}

		template<typename... Args>
		static constexpr void core_info(spdlog::format_string_t<Args...> fmt, Args&&... args) {
			if constexpr (Level::Info >= compile_time_min_level) {
				print_message_impl(Type::Core, Level::Info, fmt, std::forward<Args>(args)...);
			}
		}

		template<typename... Args>
		static constexpr void core_warn(spdlog::format_string_t<Args...> fmt, Args&&... args) {
			if constexpr (Level::Warn >= compile_time_min_level) {
				print_message_impl(Type::Core, Level::Warn, fmt, std::forward<Args>(args)...);
			}
		}

		template<typename... Args>
		static constexpr void core_error(spdlog::format_string_t<Args...> fmt, Args&&... args) {
			if constexpr (Level::Error >= compile_time_min_level) {
				print_message_impl(Type::Core, Level::Error, fmt, std::forward<Args>(args)...);
			}
		}

		template<typename... Args>
		static constexpr void core_fatal(spdlog::format_string_t<Args...> fmt, Args&&... args) {
			if constexpr (Level::Fatal >= compile_time_min_level) {
				print_message_impl(Type::Core, Level::Fatal, fmt, std::forward<Args>(args)...);
			}
		}

		// Client logging functions
		template<typename... Args>
		static constexpr void trace(spdlog::format_string_t<Args...> fmt, Args&&... args) {
			if constexpr (Level::Trace >= compile_time_min_level) {
				print_message_impl(Type::Client, Level::Trace, fmt, std::forward<Args>(args)...);
			}
		}

		template<typename... Args>
		static constexpr void info(spdlog::format_string_t<Args...> fmt, Args&&... args) {
			if constexpr (Level::Info >= compile_time_min_level) {
				print_message_impl(Type::Client, Level::Info, fmt, std::forward<Args>(args)...);
			}
		}

		template<typename... Args>
		static constexpr void warn(spdlog::format_string_t<Args...> fmt, Args&&... args) {
			if constexpr (Level::Warn >= compile_time_min_level) {
				print_message_impl(Type::Client, Level::Warn, fmt, std::forward<Args>(args)...);
			}
		}

		template<typename... Args>
		static constexpr void error(spdlog::format_string_t<Args...> fmt, Args&&... args) {
			if constexpr (Level::Error >= compile_time_min_level) {
				print_message_impl(Type::Client, Level::Error, fmt, std::forward<Args>(args)...);
			}
		}

		template<typename... Args>
		static constexpr void fatal(spdlog::format_string_t<Args...> fmt, Args&&... args) {
			if constexpr (Level::Fatal >= compile_time_min_level) {
				print_message_impl(Type::Client, Level::Fatal, fmt, std::forward<Args>(args)...);
			}
		}

		// Tagged logging functions - Core
		template<typename... Args>
		static constexpr void core_trace_tag(const std::string& tag, spdlog::format_string_t<Args...> fmt, Args&&... args) {
			if constexpr (Level::Trace >= compile_time_min_level) {
				print_message_tag_impl(Type::Core, Level::Trace, tag, fmt, std::forward<Args>(args)...);
			}
		}

		template<typename... Args>
		static constexpr void core_info_tag(const std::string& tag, spdlog::format_string_t<Args...> fmt, Args&&... args) {
			if constexpr (Level::Info >= compile_time_min_level) {
				print_message_tag_impl(Type::Core, Level::Info, tag, fmt, std::forward<Args>(args)...);
			}
		}

		template<typename... Args>
		static constexpr void core_warn_tag(const std::string& tag, spdlog::format_string_t<Args...> fmt, Args&&... args) {
			if constexpr (Level::Warn >= compile_time_min_level) {
				print_message_tag_impl(Type::Core, Level::Warn, tag, fmt, std::forward<Args>(args)...);
			}
		}

		template<typename... Args>
		static constexpr void core_error_tag(const std::string& tag, spdlog::format_string_t<Args...> fmt, Args&&... args) {
			if constexpr (Level::Error >= compile_time_min_level) {
				print_message_tag_impl(Type::Core, Level::Error, tag, fmt, std::forward<Args>(args)...);
			}
		}

		template<typename... Args>
		static constexpr void core_fatal_tag(const std::string& tag, spdlog::format_string_t<Args...> fmt, Args&&... args) {
			if constexpr (Level::Fatal >= compile_time_min_level) {
				print_message_tag_impl(Type::Core, Level::Fatal, tag, fmt, std::forward<Args>(args)...);
			}
		}

		// Tagged logging functions - Client
		template<typename... Args>
		static constexpr void trace_tag(const std::string& tag, spdlog::format_string_t<Args...> fmt, Args&&... args) {
			if constexpr (Level::Trace >= compile_time_min_level) {
				print_message_tag_impl(Type::Client, Level::Trace, tag, fmt, std::forward<Args>(args)...);
			}
		}

		template<typename... Args>
		static constexpr void info_tag(const std::string& tag, spdlog::format_string_t<Args...> fmt, Args&&... args) {
			if constexpr (Level::Info >= compile_time_min_level) {
				print_message_tag_impl(Type::Client, Level::Info, tag, fmt, std::forward<Args>(args)...);
			}
		}

		template<typename... Args>
		static constexpr void warn_tag(const std::string& tag, spdlog::format_string_t<Args...> fmt, Args&&... args) {
			if constexpr (Level::Warn >= compile_time_min_level) {
				print_message_tag_impl(Type::Client, Level::Warn, tag, fmt, std::forward<Args>(args)...);
			}
		}

		template<typename... Args>
		static constexpr void error_tag(const std::string& tag, spdlog::format_string_t<Args...> fmt, Args&&... args) {
			if constexpr (Level::Error >= compile_time_min_level) {
				print_message_tag_impl(Type::Client, Level::Error, tag, fmt, std::forward<Args>(args)...);
			}
		}

		template<typename... Args>
		static constexpr void fatal_tag(const std::string& tag, spdlog::format_string_t<Args...> fmt, Args&&... args) {
			if constexpr (Level::Fatal >= compile_time_min_level) {
				print_message_tag_impl(Type::Client, Level::Fatal, tag, fmt, std::forward<Args>(args)...);
			}
		}

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

		// Internal implementation functions
		template<typename... Args>
		static inline void print_message_impl(Type type, Level level, spdlog::format_string_t<Args...> fmt, Args&&... args) {
			auto detail = m_enabled_tags[""];
			if (detail.enabled && detail.level_filter <= level) {
				auto logger = (type == Type::Core) ? GetCoreLogger() : GetClientLogger();
				switch (level) {
					case Level::Trace: logger->trace(fmt, std::forward<Args>(args)...); break;
					case Level::Info:  logger->info(fmt, std::forward<Args>(args)...); break;
					case Level::Warn:  logger->warn(fmt, std::forward<Args>(args)...); break;
					case Level::Error: logger->error(fmt, std::forward<Args>(args)...); break;
					case Level::Fatal: logger->critical(fmt, std::forward<Args>(args)...); break;
				}
			}
		}

		template<typename... Args>
		static inline void print_message_tag_impl(Type type, Level level, const std::string& tag, spdlog::format_string_t<Args...> fmt, Args&&... args) {
			auto detail = m_enabled_tags[tag];
			if (detail.enabled && detail.level_filter <= level) {
				auto logger = (type == Type::Core) ? GetCoreLogger() : GetClientLogger();
				std::string formatted = fmt::format(fmt, std::forward<Args>(args)...);
				switch (level) {
					case Level::Trace: logger->trace("[{}] {}", tag, formatted); break;
					case Level::Info:  logger->info("[{}] {}", tag, formatted); break;
					case Level::Warn:  logger->warn("[{}] {}", tag, formatted); break;
					case Level::Error: logger->error("[{}] {}", tag, formatted); break;
					case Level::Fatal: logger->critical("[{}] {}", tag, formatted); break;
				}
			}
		}
	};

}

// Modern C++ Usage Examples:
// ignis::Log::core_info("Engine initialized successfully");
// ignis::Log::trace_tag("Renderer", "Loaded {} textures", count);
// ignis::Log::error("Failed to load file: {}", filename);
